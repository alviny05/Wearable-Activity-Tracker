/*
This file contains the code for initializing and connecting to a WiFi Station
Justin Nascimento (U42983905) and Alvin Yan ()
*/

#ifndef WIFI_H
#define WIFI_H


// Connection to WiFi Libraries
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

// UDP Messaging Libraries
#include <sys/param.h>
#include <lwip/netdb.h>
#include "esp_netif.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"


// Connection to WiFi Config Variables and Defines
#define EXAMPLE_ESP_WIFI_SSID      "Group_4"
#define EXAMPLE_ESP_WIFI_PASS      "smartsys"
#define EXAMPLE_ESP_MAXIMUM_RETRY  5
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

// UDP Definitions
#define HOST_IP_ADDR "192.168.1.119"        // TODO - UPDATE
#define PORT 3333

// Struct definition
struct udp_data {
    int sock;
    struct sockaddr_in dest_addr;
};

// Connection to WiFi Function Prototypes
void wifi_init_sta(void);
void init_udp(struct udp_data* udpData);

#endif