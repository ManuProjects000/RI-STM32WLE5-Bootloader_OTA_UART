/**
 * @file r_flash_addresses.h
 * @brief Defines flash memory addresses and sizes for application firmware and EEPROM emulation.
 *
 * @details
 * This file specifies the flash memory layout for:
 * - Application Bank A,
 * - EEPROM emulation storage,
 * - Real flash start address reference.
 *
 * These definitions are used by the bootloader and application to 
 * properly handle firmware storage, updates, and EEPROM emulation.
 */
 
 
#ifndef R_FLASH_HEADERS_H
#define R_FLASH_HEADERS_H

/**
 * @brief Start address of Application Bank A in flash memory.*/
#define APP_A_ADDRESS 				0x08004000UL

/**
 * @brief Address reserved for EEPROM emulation in flash.*/
#define EEPROM_ADDRESS  			0x0803E800UL

//LoRaWAN NVM -> 0x0803F000UL

/**
 * @brief Maximum application size for firmware bank.
 * Calculated as half the size between APP_A_ADDRESS and EEPROM_ADDRESS*/
#define APP_MAX_SIZE 					(EEPROM_ADDRESS - APP_A_ADDRESS)

/**
 * @brief Start address of the real flash memory.*/
#define REAL_FLASH_START			0x08000000UL

#endif // R_FLASH_HEADERS_H