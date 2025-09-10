/**
 * @file r_flash_addresses.h
 * @brief Defines flash memory addresses and sizes for application firmware and EEPROM emulation.
 *
 * @details
 * This file specifies the flash memory layout for:
 * - Application Bank A and Bank B,
 * - Free Page that couldnt be assigned to a Bank,
 * - EEPROM emulation storage,
 * - Real flash start address reference.
 *
 * These definitions are used by the bootloader and application to 
 * properly handle firmware storage, updates, and EEPROM emulation.
 */
 
 
#ifndef R_FLASH_HEADERS_H
#define R_FLASH_HEADERS_H

/**
 * @brief Maximum application size for each firmware bank.
 * Calculated as half the size between APP_A_ADDRESS and FREE_PAGE.*/
#define APP_MAX_SIZE ((FREE_PAGE - APP_A_ADDRESS) / 2)
//0x1E000UL --> 60 pages = 122,880 bytes

/**
 * @brief Start address of Application Bank A in flash memory.*/
#define APP_A_ADDRESS 0x08004000UL

/**
 * @brief Start address of Application Bank B in flash memory.
 * Located immediately after Bank A with size APP_MAX_SIZE.*/
#define APP_B_ADDRESS	(APP_A_ADDRESS + APP_MAX_SIZE) 
//APP B: 0x08020000

/**
 * @brief Address of the free page. Couldnt be assigned to a Bank*/
#define FREE_PAGE 		0x0803E000UL

/**
 * @brief Address reserved for EEPROM emulation in flash.*/
#define EEPROM_ADDRESS  			0x0803E800UL

//LoRaWAN NVM -> 0x0803F000UL

/**
 * @brief Start address of the real flash memory.*/
#define REAL_FLASH_START			0x08000000

#endif // R_FLASH_HEADERS_H