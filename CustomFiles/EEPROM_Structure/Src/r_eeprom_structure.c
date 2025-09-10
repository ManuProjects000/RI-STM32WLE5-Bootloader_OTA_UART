/**
 * @file r_eeprom_structure.c
 * @brief EEPROM emulation structure and function prototypes.
 *
 * @author Manuel Martinez Leanes
 * @date 22/08/2025
 *
 * Structure and functions used for EEPROM emulation
 * in flash memory. It provides mechanisms to initialize, read, write, and 
 * update emulated EEPROM data, including firmware update flags and metadata.
 *
 * @details
 * The emulated EEPROM stores:
 * - A magic number for validation,
 * - Update and block-update flags,
 * - Firmware version,
 * - Firmware size received,
 * - Firmware CRC.
 *
 * These values are used during the firmware update and bootloader processes
 * to validate firmware integrity and manage update states.
 */

#include "string.h"

#include "r_eeprom_structure.h"

//Init EEPROM if not initialized
void 
r_init_eeprom_if_needed(void) 
{
    EEPROM_Emu_Data ee_current = r_read_eeprom_data();
    if (ee_current.magic != EEPROM_MAGIC) 
		{
        EEPROM_Emu_Data ee_defaults = 
				{
            .magic = EEPROM_MAGIC,
            .flag_update = FLAG_VALUE_FALSE,
            .flag_block_updates = FLAG_VALUE_FALSE,
            .version = 0,
						.fw_received_size = 0,
						.fw_crc = 0
        };
        r_write_eeprom_data(&ee_defaults);
    }
}

//Get EEPROM Values from Flash
EEPROM_Emu_Data 
r_read_eeprom_data(void) 
{
    EEPROM_Emu_Data ee_data;
    memcpy(&ee_data, (void*)EEPROM_ADDRESS, sizeof(EEPROM_Emu_Data));
    return ee_data;
}

//Write EEPROM new Values in Flash
void 
r_write_eeprom_data(EEPROM_Emu_Data* ee_new_data) 
{
    HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef erase;
    uint32_t page_error = 0;

    erase.TypeErase = FLASH_TYPEERASE_PAGES;
    erase.Page = (EEPROM_ADDRESS - REAL_FLASH_START) / FLASH_PAGE_SIZE;
    erase.NbPages = 1;

    HAL_FLASHEx_Erase(&erase, &page_error);

    uint64_t* p_data = (uint64_t*)ee_new_data;
    for (uint32_t i = 0; i < sizeof(EEPROM_Emu_Data)/8; i++) 
		{
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, EEPROM_ADDRESS + i * 8, p_data[i]);
    }

    HAL_FLASH_Lock();
}

//Set EEPROM new Values
void 
r_set_eeprom_flags(uint32_t flag_up, uint32_t flag_bu, uint32_t version, uint32_t rs, uint32_t crc)
{
	EEPROM_Emu_Data ee_current = r_read_eeprom_data();
	if(flag_up != 0xFFFFFFFF)
	{
			ee_current.flag_update = flag_up;
	}
	if(flag_bu != 0xFFFFFFFF)
	{
			ee_current.flag_block_updates = flag_bu;
	}
	if(version != 0xFFFFFFFF)
	{
			ee_current.version = version;
	}
	if(rs != 0xFFFFFFFF)
	{
			ee_current.fw_received_size = rs;
	}
	if(crc != 0xFFFFFFFF)
	{
			ee_current.fw_crc = crc;
	}
	r_write_eeprom_data(&ee_current);
}