/**
 * @file r_flash_functions.c
 * @brief Flash memory utility functions for STM32.
 *
 * @author Manuel Martinez Leanes
 * @date 25/08/2025
 *
 * Functions to program, erase, and swap flash memory
 * pages and banks. It is primarily used in firmware update and bootloader
 * contexts, where handling of dual-bank memory is required.
 *
 * @details
 * The provided functions include:
 * - Page-level programming (`r_flash_program_page`)
 * - Page erasing (`r_flash_erase_page`)
 * - Page swapping (`r_flash_swap_pages`)
 * - Bank swapping (`r_flash_swap_bank`)
 *
 * These utilities use the STM32 HAL API for flash operations and follow
 * STM32 constraints, such as programming in double words (64-bit).
 */

#include "r_flash_functions.h"

// Start PAGE FUNCTIONALITY -------------------------------------------------------------------------------------------

void 
r_flash_program_page(uint32_t address, uint8_t* data) 
{
    for (uint32_t i = 0; i < FLASH_PAGE_SIZE; i += 8) 
		{
        uint64_t data64 = 0;
        memcpy(&data64, data + i, sizeof(uint64_t));
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address + i, data64);
    }
}

void 
r_flash_erase_page(uint32_t address) 
{
    FLASH_EraseInitTypeDef erase;
    uint32_t pageError;

    erase.TypeErase = FLASH_TYPEERASE_PAGES;
    erase.Page = (address - FLASH_BASE) / FLASH_PAGE_SIZE;
    erase.NbPages = 1;

    HAL_FLASHEx_Erase(&erase, &pageError);
}

void 
r_flash_swap_pages(uint32_t addrA, uint32_t addrB) 
{
    HAL_FLASH_Unlock();

		// Erase destination page
    r_flash_erase_page(addrA);
	
		// Program destination page with contents from source
    r_flash_program_page(addrA, (uint8_t*)addrB);

    HAL_FLASH_Lock();
}

// End PAGE FUNCTIONALITY ---------------------------------------------------------------------------------------------

// Start BANK FUNCTIONALITY -------------------------------------------------------------------------------------------

void 
r_flash_swap_bank(uint32_t baseA, uint32_t baseB, uint32_t num_pages) 
{
    for (uint32_t i = 0; i < num_pages; i++) 
		{
			// Visual indication: RED led
				HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_9);
				HAL_Delay(50);
				HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_9);
			
				// Calculate page addresses for Bank A and Bank B
        uint32_t addrA = baseA + i * FLASH_PAGE_SIZE;
        uint32_t addrB = baseB + i * FLASH_PAGE_SIZE;
			
				// Perform page swap
        r_flash_swap_pages(addrA, addrB);
    }
}

// End BANK FUNCTIONALITY ---------------------------------------------------------------------------------------------