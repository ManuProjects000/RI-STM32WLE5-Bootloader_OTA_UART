/**
 * @file r_task_led.c
 * @brief Implementation of the LED signaling task for RTOS.
 *
 * @author Manuel Martinez Leanes
 * @date 22/08/2025
 *
 * @details
 * LED task that waits for FLAG_TX and
 * blinks a GPIO LED. It uses FreeRTOS and CMSIS-OS task mechanisms.
 */

#include "r_task_led.h"

void 
r_led_task(void *argument)
{
	uint32_t flags;
	for(;;)
	{
			flags = osThreadFlagsWait(FLAG_TX , osFlagsWaitAny, osWaitForever);
			
			if (flags & FLAG_TX)
			{
					// Acción cuando se transmitió algo (ej: prender LED verde)
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);  // LED TX
					osDelay(200);
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
			}
	}
}