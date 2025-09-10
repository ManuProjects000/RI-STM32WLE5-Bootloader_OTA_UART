/**
 * @file r_task_led.h
 * @brief LED signaling task definitions for RTOS.
 *
 * @author Manuel Martinez Leanes
 * @date 22/08/2025
 *
 */
 
#ifndef LED_TASK_H
#define LED_TASK_H
 
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "gpio.h"
#include "r_flags.h"

/**
 * @brief LED signaling task. 
 * Waits for FLAG_TX and blinks a GPIO LED.
 * @param argument Not used.
 */
void r_led_task(void *argument);

#endif // LED_TASK_H