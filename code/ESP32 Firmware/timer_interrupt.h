/*
This file contains the code for Initializing the on-board timer for Hardware Interrupts
Justin Nascimento (U42983905) and Alvin Yan ()
*/

#ifndef TIMER_INTERRUPT_H
#define TIMER_INTERRUPT_H

#include <stdio.h>
#include <string.h>
#include "driver/i2c.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gptimer.h"
#include "freertos/semphr.h"

// Hardware interrupt definitions
#define TIMER_INTERVAL_SEC    0.01
#define GPIO_INPUT_IO_1       4
#define ESP_INTR_FLAG_DEFAULT 0

// Global Variables
extern QueueHandle_t timer_queue;               // Queue for the Timer Task - Indicates when the timer will go off

// Function Prototypes
void timer_init();

#endif