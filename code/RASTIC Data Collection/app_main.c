#include "jsmn.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"

#include "WiFi.h"
#include "i2c_functions.h"
#include "ADXL343.h"

#include "mqtt_client.h"

#include "lwip/err.h"
#include "lwip/sys.h"

static const char *TAG = "MAIN";

// Defines Data Structure to hold the message being sent
struct FinalMoCapData{
    float accelX;
    float accelY;
    float accelZ;
    float MoCapX;
    float MoCapY;
    float MoCapZ;
};

// Defines Global Vars to hold MoCap Data and Acceleration Data
struct FinalMoCapData moCapData;
struct accelerationVals accelData;

// Defines Mutexes to avoid race conditions
static SemaphoreHandle_t accel_mutex;



// --------------------------------------------- MQTT ----------------------------------------------------//

// MQTT Variables
static int subscribed = 0;
static esp_mqtt_client_handle_t client;

// Helper: convert substring to double
float tok2float(const char *json, jsmntok_t *tok) {
    char buf[32];
    int len = tok->end - tok->start;
    if(len >= sizeof(buf)) len = sizeof(buf)-1;
    memcpy(buf, json + tok->start, len);
    buf[len] = '\0';
    return strtof(buf, NULL);  // convert string to float
}

// Parse pos array
void parse_pos(const char *payload, int len) {
    char buf[len+1];
    memcpy(buf, payload, len);
    buf[len] = '\0';  // null terminate

    jsmn_parser parser;
    jsmntok_t tokens[32];  // adjust if your JSON is bigger
    jsmn_init(&parser);
    int ret = jsmn_parse(&parser, buf, len, tokens, 32);

    if(ret < 0) {
        printf("JSON parse failed\n");
        return;
    }

    for(int i=1; i<ret; i++) {
        if(tokens[i].type == JSMN_STRING &&
           strncmp(buf + tokens[i].start, "pos", tokens[i].end - tokens[i].start) == 0) {

            jsmntok_t *posArray = &tokens[i+1];
            if(posArray->type == JSMN_ARRAY && posArray->size == 3) {
                moCapData.MoCapX = tok2float(buf, &tokens[i+2]);
                moCapData.MoCapY = tok2float(buf, &tokens[i+3]);
                moCapData.MoCapZ = tok2float(buf, &tokens[i+4]);
            }
            break; // we only care about pos
        }
    }
}

// MQTT Event Handler
static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                               int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;

    switch ((esp_mqtt_event_id_t) event_id) {

        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT Connected");
            if(!subscribed){
                esp_mqtt_client_subscribe(client, "rb/group-4", 0);
                subscribed = 1;
            }

            break;

        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT Data Received");

            // Lock Mutex
            xSemaphoreTake(accel_mutex, portMAX_DELAY);

            // Adds Accel Data to struct
            moCapData.accelX = accelData.accelX;
            moCapData.accelY = accelData.accelY;
            moCapData.accelZ = accelData.accelZ;

            // Gives Accel Mutex Back
            xSemaphoreGive(accel_mutex);

            char msg[128];

            // Add mocap data
            parse_pos(event->data, event->data_len);

            snprintf(msg, sizeof(msg),
                    "%f,%f,%f,%f,%f,%f",
                    moCapData.accelX,
                    moCapData.accelY,
                    moCapData.accelZ,
                    moCapData.MoCapX,
                    moCapData.MoCapY,
                    moCapData.MoCapZ);

            esp_mqtt_client_publish(client, "esp32/group4/mocap", msg, 0, 0, 0);

            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT Disconnected");
            break;

        default:
            break;
    }
}

// Initialize MQTT Connection
static void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtt://rasticvm.internal:1883",
        .session.protocol_ver = MQTT_PROTOCOL_V_3_1_1,
        .credentials.client_id = "esp32_group_4",
        .buffer.size = 1024
    };

    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}

// --------------------------------------------- RTOS Tasks --------------------------------------------//

// RTOS Task to Collect Acceleration Data every 10ms
static void collect_accel_task(void *arg){

    // Define temp acceleration structure to collect acceleration values
    struct accelerationVals tempAccel;

    while(1){

        // Collect + Filter Acceleration Data
        tempAccel = getAccel();

        accelerationThresholdFilter(&tempAccel);

        // Write tempAccel to Accel data, using mutex to block
        xSemaphoreTake(accel_mutex, portMAX_DELAY);

        accelData.accelX = tempAccel.accelX;
        accelData.accelY = tempAccel.accelY;
        accelData.accelZ = tempAccel.accelZ;


        xSemaphoreGive(accel_mutex);

        vTaskDelay(10 / portTICK_PERIOD_MS); // keeps Watchdog from idling
    }
}


/* ================= MAIN ================= */

void app_main(void)
{
    // Initializes Mutexes
    accel_mutex = xSemaphoreCreateMutex();

    // Initialize I2C Communication and Accelerometer
    i2c_master_init();
    i2c_scanner();
    initializeADXL343();

    // Initialize WiFi
    wifi_init_sta();

    // Initialize MQTT
    mqtt_app_start();

    // Initializes RTOS Task to collect accel data
    xTaskCreate(collect_accel_task, "collect_accel_task", 2000, NULL, 5, NULL);
}