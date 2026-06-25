/*
This file contains the code for initializing and connecting to a WiFi Station
Justin Nascimento (U42983905) and Alvin Yan ()
*/

#ifndef WIFI_H
#define WIFI_H

// WiFi Libraries
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

// WiFi Config Variables and defines
#define EXAMPLE_ESP_WIFI_SSID      "ee444"
#define EXAMPLE_ESP_WIFI_PASS      "sunnyboston"
#define EXAMPLE_ESP_MAXIMUM_RETRY  5
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

// Function Prototypes
void wifi_init_sta(void);

#endif