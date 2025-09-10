/**
 * @file r_eeprom_structure.h
 * @brief EEPROM emulation structure and function prototypes.
 *
 * @author Manuel Martinez Leanes
 * @date 22/08/2025
 *
 */

#ifndef R_EEPROM_STRUCTURE_H
#define R_EEPROM_STRUCTURE_H

#include "main.h"
#include "stdint.h"

#include "r_flash_addresses.h"

#define FLAG_VALUE_TRUE				0
#define FLAG_VALUE_FALSE	  	1

#define EEPROM_MAGIC					0x4ED177EC

typedef struct 
{
		uint32_t magic;
    uint32_t flag_update;
    uint32_t flag_block_updates;
    uint32_t version;
		uint32_t fw_received_size;
		uint32_t fw_crc;
} EEPROM_Emu_Data;

/**
 * @brief Initialize EEPROM emulation data if not already initialized.
 * Reads current EEPROM emulation data and checks the magic value.
 * If the magic value does not match, writes default values to EEPROM.
 */
void r_init_eeprom_if_needed(void);

/**
 * @brief Reads the EEPROM emulation data.
 * Copies the data stored at EEPROM_ADDRESS into a local structure.
 * 
 * @return EEPROM_Emu_Data The current EEPROM emulation data.
 */
EEPROM_Emu_Data r_read_eeprom_data(void);


/**
 * @brief Writes new data to the EEPROM emulation.
 * Unlocks flash, erases the EEPROM page, programs the new data, and locks flash again.
 * 
 * @param[in] new_data Pointer to the EEPROM_Emu_Data structure to write.
 */
void r_write_eeprom_data(EEPROM_Emu_Data* ee_new_data);

/**
 * @brief Updates EEPROM flags and firmware info if provided.
 * 
 * Reads current EEPROM data, updates fields only if parameters are not zero,
 * and writes back the updated data.
 * 
 * @param[in] flag_up       Update flag value (set if non-zero).
 * @param[in] flag_bu       Block updates flag value (set if non-zero).
 * @param[in] version       Firmware version value (set if non-zero).
 * @param[in] rs            Firmware received size (set if non-zero).
 * @param[in] crc           Firmware CRC value (set if non-zero).
 */
void r_set_eeprom_flags(uint32_t flag_up, uint32_t flag_bu, uint32_t version, uint32_t rs, uint32_t crc);

#endif // R_EEPROM_STRUCTURE_H