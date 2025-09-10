/**
 * @file r_flash_functions.h
 * @brief Flash memory utility functions for STM32.
 *
 * @author Manuel Martinez Leanes
 * @date 25/08/2025
 *
 */

#ifndef R_FLASH_FUNCTIONS_H
#define R_FLASH_FUNCTIONS_H

#include "main.h"
#include "string.h"
#include "stdint.h"

/**
 * @brief Program a flash page with provided data.
 *
 * This function writes an entire flash page at the specified address.
 * Data is programmed in 64-bit (double word) chunks, as required by 
 * the STM32 flash programming interface.
 *
 * @param address  Starting address of the flash page to program.
 * @param data     Pointer to the buffer containing data to be written.
 */
void r_flash_program_page(uint32_t address, uint8_t* data);

/**
 * @brief Erase a single flash page.
 *
 * This function erases the flash page that contains the given address.
 *
 * @param address Address within the flash page to erase.
 */
void r_flash_erase_page(uint32_t address);

/**
 * @brief Swap contents between two flash pages.
 *
 * This function erases the destination page and copies the contents 
 * of the source page into it.
 *
 * @param addrA Address of the destination page (target).
 * @param addrB Address of the source page (to be copied).
 */
void r_flash_swap_pages(uint32_t addrA, uint32_t addrB);


/**
 * @brief Swap contents of two flash banks.
 *
 * This function iterates over all pages in Bank A and Bank B,
 * erasing each page of Bank A and rewriting it with the corresponding 
 * page from Bank B. A small GPIO toggle delay is used as a visual 
 * indication of progress.
 *
 * @param baseA      Base address of Bank A (destination).
 * @param baseB      Base address of Bank B (source).
 * @param num_pages  Number of flash pages to swap.
 */
void r_flash_swap_bank(uint32_t baseA, uint32_t baseB, uint32_t num_pages);

#endif // R_FLASH_FUNCTIONS_H