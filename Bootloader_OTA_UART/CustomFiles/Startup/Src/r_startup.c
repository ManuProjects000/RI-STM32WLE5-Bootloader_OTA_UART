/**
 * @file r_startup.c
 * @brief Bootloader startup and application jump routines.
 *
 * @author Manuel Martinez Leanes
 * @date 25/08/2025
 *
 * This header defines the core functions for the bootloader startup
 * process and for transferring execution control to the main user 
 * application. It ensures that system peripherals are properly reset, 
 * memory is validated, and firmware updates are handled before jumping 
 * to the application.
 *
 * @details
 * Key functions provided:
 * - `r_go_to_app()`: De-initialize system and jump to application reset handler.
 * - `r_startup_routine()`: Bootloader startup sequence that verifies update 
 *   flags, handles bank swaps, performs CRC validation, and decides the 
 *   execution path (update or run application).
 */

#include "r_startup.h"

#define APP_NUM_PAGES (APP_MAX_SIZE / FLASH_PAGE_SIZE)

uint32_t crc = 0;

void r_led_burst()
{
	HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_9);
	HAL_Delay(200);
	HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_9);
	HAL_Delay(200);
	HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_9);
	HAL_Delay(200);
	HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_9);
	HAL_Delay(200);
	HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_9);
	HAL_Delay(200);
	HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_9);
	HAL_Delay(200);
}

// Start GO TO APP ----------------------------------------------------------------------------------------------------
void 
r_go_to_app(uint32_t app_address)
{
	
	// De-initialize peripherals and release GPIO resources if required
	HAL_GPIO_DeInit(GPIOB, GPIO_PIN_5);		// Release pin PA5
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9);
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_0);
	__HAL_RCC_GPIOA_CLK_DISABLE();  			// Disable GPIOA clock
	__HAL_RCC_GPIOB_CLK_DISABLE();  			// Disable GPIOA clock
	
	// Reset clock configuration and HAL state
	HAL_RCC_DeInit();
  HAL_DeInit();
	
	// Disable SysTick timer to avoid unwanted interrupts during app execution
	SysTick->CTRL = 0;
	SysTick->LOAD = 0;
	SysTick->VAL  = 0;
	
	// Disable all interrupts
	__disable_irq();						
	
	// Relocate vector table to the application's address
	SCB->VTOR = app_address;		
	
	// Fetch application's Reset Handler address from vector table
	void (*app_reset_handler)(void) = (void (*) (void)) (*(volatile uint32_t*)(app_address + 4U));
	
	// Jump to application's Reset Handler
	app_reset_handler();
}
// End GO TO APP ------------------------------------------------------------------------------------------------------
