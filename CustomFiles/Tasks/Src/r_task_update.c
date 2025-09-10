/**
 * @file r_task_update.c
 * @brief FreeRTOS OTA firmware update task and flash handling.
 *
 * @author Manuel Martinez Leanes
 * @date 22/08/2025
 *
 * @details
 * This file implements the Over-The-Air (OTA) firmware update mechanism
 * for STM32 microcontrollers using FreeRTOS tasks. It manages:
 * - Receiving firmware packets from LoRa or another transport.
 * - Storing firmware in Bank B flash memory.
 * - Handling remnant bytes when packet size does not align with flash page size.
 * - Verifying CRC checksums for each page and the full firmware.
 * - Updating EEPROM emulation flags to indicate new firmware availability.
 *
 * The file contains:
 * - Volatile variables used for inter-task communication and OTA state.
 * - Static buffers for page processing and remnant storage.
 * - Private helper functions for flash erase/program operations.
 * - A FreeRTOS task (`receive_update`) that orchestrates the OTA process.
 *
 * Flash memory layout follows the definitions in `r_flash_addresses.h`.
 * CRC calculations rely on `r_crc.h`.
 */

#include "r_task_update.h"

// Start VOLATILE Variables -------------------------------------------------------------------------------------------
/**
 * @brief Raw receive buffer for incoming OTA packets.
 */
volatile uint8_t g_rx_buffer[ETX_OTA_PACKET_MAX_SIZE] = {0};

/**
 * @brief Total firmware size (in bytes) expected during OTA.
 */
volatile uint32_t g_ota_fw_total_size = 0;

/**
 * @brief Expected CRC32 checksum of the incoming firmware.
 */
volatile uint32_t g_ota_fw_crc = 0;

/**
 * @brief Total number of firmware bytes received so far.
 */
volatile uint32_t g_ota_fw_received_size = 0;

/**
 * @brief OTA state machine current state.
 */
volatile ETX_OTA_STATE_ g_ota_state = ETX_OTA_STATE_IDLE;

/**
 * @brief CRC32 checksum for the current OTA bulk transfer.
 *
 * This variable stores the CRC value of the currently processed bulk
 * of firmware data during an OTA update. It is updated when a bulk
 * header packet is received and used to verify the integrity of the
 * firmware data before writing it to flash memory.
 *
 */
volatile uint32_t g_ota_bulk_crc;
// End VOLATILE Variables ---------------------------------------------------------------------------------------------



// Start STATIC Variables ---------------------------------------------------------------------------------------------

// End STATIC Variables -----------------------------------------------------------------------------------------------



// Start Private function prototypes ----------------------------------------------------------------------------------

// End Private function prototypes ------------------------------------------------------------------------------------



// ====================================================================================================================
// Start Update FreeRTOS TASK -----------------------------------------------------------------------------------------
void 
r_receive_update(void *argument)
{
	for(;;)
	{
		osThreadFlagsWait(FLAG_RECEIVE_UPDATE, osFlagsWaitAny, osWaitForever);
		
		
		ETX_OTA_COMMAND_ *cmd = (ETX_OTA_COMMAND_*)g_rx_buffer;
				
		if (cmd->packet_type == ETX_OTA_PACKET_TYPE_CMD)
		{
			if(cmd->cmd == ETX_OTA_CMD_START)
			{
				r_set_eeprom_flags(FLAG_VALUE_TRUE, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
					
				osDelay(2000);
				__disable_irq();
				NVIC_SystemReset();
			}
		}
	}
}
// End Update FreeRTOS TASK -------------------------------------------------------------------------------------------
// ====================================================================================================================
