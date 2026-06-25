/*
This file contains the main ESP32 code for Quest 3, collecting accelerometer data and
transmitting it over UDP to the Raspberry Pi.
Justin Nascimento (U42983905) and Alvin Yan ()
*/


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
#include "timer_interrupt.h"



// Definitions and Global Variables -----------------------------------------------------
static const char *TAG = "MAIN";


// UDP Global Variables
char udp_payload[128];
struct udp_data udpData;

// Main RTOS Task - Collects accelerometer data and transmits it every 10ms
static void main_task(void *arg){
    int evt;
    int err;

    // Initialize Struct to store sensor data
    struct accelerationVals accelerationReading;

    // On the timer, collect and send data over UART to node.js
    while(1){
        // On the timer, collect data
        if (xQueueReceive(timer_queue, &evt, portMAX_DELAY)) {

            // Collect Acceleration Data
            accelerationReading = getAccel();

            // Log Collected Data

            ESP_LOGD(TAG, "%f %f %f\n", accelerationReading.accelX, accelerationReading.accelY, accelerationReading.accelZ);

            // Send a UDP Message with the acceleration values
            memset(udp_payload, 0, sizeof(udp_payload));
            snprintf(udp_payload, sizeof(udp_payload), "%f %f %f\n", accelerationReading.accelX, accelerationReading.accelY, accelerationReading.accelZ);

            err = sendto(udpData.sock, udp_payload, strlen(udp_payload), 0, (struct sockaddr *)&(udpData.dest_addr), sizeof(udpData.dest_addr));
            if (err < 0) {
                ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
            }
            ESP_LOGI(TAG, "Message sent");

            vTaskDelay(pdMS_TO_TICKS(1));
        }
    }
}





void app_main(void)
{
    // Initialize the hardware timer
    timer_init();

     // Initializes I2C communication with alphanumeric Display
    i2c_master_init();
    i2c_scanner();

    // Initializes Accelerometer
    initializeADXL343();

    // Connect to WiFi
    wifi_init_sta();

    // Initialize UDP Connection
    init_udp(&udpData);

    vTaskDelay(pdMS_TO_TICKS(1000)); // 1 second

    // Initializes the task to collect accelerometer data and send it over UDP to the Router
    xTaskCreate(main_task, "main_task", 4096, NULL, 5, NULL);
}
