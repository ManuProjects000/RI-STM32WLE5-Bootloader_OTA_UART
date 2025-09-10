/**
 * @file r_startup.h
 * @brief Bootloader startup and application jump routines.
 *
 * @author Manuel Martinez Leanes
 * @date 25/08/2025
 *
 */

#ifndef R_STARTUP_H
#define R_STARTUP_H

#include "main.h"
#include "r_flash_addresses.h"
#include "r_eeprom_structure.h"
#include "r_crc.h"
#include "r_flash_functions.h"
#include "r_routine_update.h"

extern uint32_t crc;

void r_led_burst();

/**
 * @brief Jump execution to the user application.
 *
 * This function de-initializes peripherals, disables interrupts,
 * reconfigures the vector table to point to the user application,
 * and finally jumps to the application's reset handler.
 *
 * @param app_address Base address of the application in flash memory.
 */
void r_go_to_app(uint32_t app_address);

/**
 * @brief Bootloader startup routine.
 *
 * This function runs during the bootloader startup phase. It performs
 * initial signaling with GPIO, verifies the EEPROM status flags, and
 * decides whether to:
 * - Copy Bank B into Bank A (update process),
 * - Block future updates if copying fails,
 * - Or jump directly to the application in Bank A.
 *
 * Update validation is performed by calculating the CRC of the firmware
 * after copying and comparing it with the expected value stored in EEPROM.
 */
void r_startup_routine();

#endif // R_STARTUP_H