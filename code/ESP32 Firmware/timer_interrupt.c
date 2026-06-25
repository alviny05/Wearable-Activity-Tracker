/*
This file contains the code for Initializing the on-board timer for Hardware Interrupts
Justin Nascimento (U42983905) and Alvin Yan ()
*/

#include "timer_interrupt.h"

// Global Variables
static gptimer_handle_t gptimer;         // Timer
QueueHandle_t timer_queue;               // Queue for the Still Task - Indicates when the timer will go off to update display

// Timer interrupt handler -- add to timer queue
static bool IRAM_ATTR timer_callback(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)
{
    int evt = 1;
    BaseType_t high_task_wakeup = pdFALSE;

    xQueueSendFromISR(timer_queue, &evt, &high_task_wakeup);

    return high_task_wakeup == pdTRUE;
}

// Configure timer
static void timer_init_gptimer(gptimer_handle_t gptimer)
{
    // Sets timer settings
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1 * 1000 * 1000,  // 1 MHz
    };

    // Initializes new timer
    gptimer_new_timer(&timer_config, &gptimer);

    // Sets callbacks for timer and Registers them
    gptimer_event_callbacks_t cbs = {
        .on_alarm = timer_callback,
    };
    gptimer_register_event_callbacks(gptimer, &cbs, NULL);

    // Sets alarm settings and sets the alarm
    gptimer_alarm_config_t alarm_config = {
        .alarm_count = TIMER_INTERVAL_SEC * 1000000,
        .reload_count = 0,
        .flags.auto_reload_on_alarm = true,
    };
    gptimer_set_alarm_action(gptimer, &alarm_config);

    gptimer_enable(gptimer);
    gptimer_start(gptimer);
}

void timer_init(){
    timer_queue = xQueueCreate(10, sizeof(int));
    timer_init_gptimer(gptimer);
}