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

#include "r_routine_update.h"

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
/**
 * @brief Flag indicating if CRC verification succeeded.
 */
static uint8_t s_flag_flash_crc_ok = 0;

/**
 * @brief Current offset within the flash page buffer.
 */
static uint16_t s_page_offset = 0;

/**
 * @brief CRC32 of the currently buffered flash page.
 *
 * This variable stores the CRC value computed over the current
 * flash page (`page_buffer`) before it is programmed into memory.
 * Used to verify page integrity during OTA firmware updates.
 */
static uint32_t s_pagecrc = 0;

/**
 * @brief Accumulated CRC32 of the entire firmware written to flash.
 *
 * This variable stores the CRC32 value calculated over all the
 * firmware data programmed into flash (Bank B). It is used to
 * validate the firmware integrity after the OTA update.
 */
static uint32_t s_flash_crc = 0;

/**
 * @brief Current write index in Bank B flash memory.
 */
static uint32_t s_bank_a_index = APP_A_ADDRESS;

/**
 * @brief Temporary buffer for one flash page of firmware data.
 */
static uint8_t s_page_buffer[FLASH_PAGE_SIZE] = {0};

extern UART_HandleTypeDef *p_uart;

extern volatile uint8_t s_packet_ready;
// End STATIC Variables -----------------------------------------------------------------------------------------------



// Start Private function prototypes ----------------------------------------------------------------------------------
/**
 * @brief Erase all pages corresponding to Bank B firmware region.
 * @return HAL status.
 */
static HAL_StatusTypeDef r_clean_bank (void);

/**
 * @brief Program a full flash page with data from a buffer.
 * @param address Starting flash address to write.
 * @param data    Pointer to the page data (must be FLASH_PAGE_SIZE bytes).
 * @return HAL status.
 */
static HAL_StatusTypeDef r_flash_program_page(uint32_t address, uint8_t* data);

/**
 * @brief Process a received firmware packet.
 *
 * This function:
 *   - Merges leftover bytes from previous packet (if any).
 *   - Stores packet data in page_buffer.
 *   - When page_buffer is full, writes it to flash.
 *   - Handles remnant bytes for next iteration.
 *
 * @param data      Pointer to packet payload.
 * @param data_len  Number of bytes in the packet payload.
 * @return HAL status.
 */
static HAL_StatusTypeDef r_flash_process_data(uint8_t *data, uint16_t data_len);

/**
 * @brief Write the last incomplete page to flash.
 * @return HAL status.
 */
static HAL_StatusTypeDef r_flash_program_last_data();

/**
 * @brief Process an OTA packet depending on the OTA state machine.
 * @return ETX_OTA_EX_OK on success, ETX_OTA_EX_ERR on failure.
 */
static ETX_OTA_EX_ r_process_pack(void);
// End Private function prototypes ------------------------------------------------------------------------------------


// ====================================================================================================================
// Start Update FreeRTOS TASK -----------------------------------------------------------------------------------------
void 
r_receive_update()
{
		
	
		if(g_ota_state == ETX_OTA_STATE_IDLE)
		{
			
			ETX_OTA_HEADER_ *header = (ETX_OTA_HEADER_*)g_rx_buffer;
			if(header->packet_type == ETX_OTA_PACKET_TYPE_HEADER)
			{
				EEPROM_Emu_Data ee_flags = r_read_eeprom_data();
				if(ee_flags.flag_block_updates == FLAG_VALUE_FALSE)
				{
					meta_info md = header->meta_data;
					if(md.package_size <= APP_MAX_SIZE)
					{
						g_ota_fw_total_size    = 0u;
						g_ota_fw_received_size = 0u;
						g_ota_fw_crc           = 0u;
						s_flag_flash_crc_ok 	 = 0;
						g_ota_state = ETX_OTA_STATE_HEADER;
					}
				}
			}
		}
		
		ETX_OTA_EX_ status = ETX_OTA_EX_ERR;
		
		if(g_ota_state != ETX_OTA_STATE_IDLE)
		{
			status = r_process_pack();
			memset_s((void*)g_rx_buffer, 0, ETX_OTA_PACKET_MAX_SIZE/2);
			memset_s((void*)g_rx_buffer + ETX_OTA_PACKET_MAX_SIZE/2, 0, (ETX_OTA_PACKET_MAX_SIZE/2)-1);
		}
			
		if(status == ETX_OTA_EX_OK)
		{
			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_5);
			HAL_Delay(50);
			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_5);
			if(s_flag_flash_crc_ok)
			{
				HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_5);
				HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0);
				HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_9);
				
				r_set_eeprom_flags(FLAG_VALUE_FALSE, 0xFFFFFFFF, 0xFFFFFFFF, g_ota_fw_received_size, g_ota_fw_crc);
				
				HAL_Delay(2000);
				__disable_irq();
				NVIC_SystemReset();
			}
			else
			{
				//Si llego aca es porque algo se escribio mal en la Flash, tengo que pedir el FW nuevamente.
				/**TO DO: comunicarse con el ESP32 para enviar otra vez el FW*/
				
				g_ota_state = ETX_OTA_STATE_IDLE;
			}
			
		} else
		{
			HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_9);
			HAL_Delay(50);
			HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_9);
			
			const uint8_t msg[] = "NACK\n";
			HAL_UART_Transmit(p_uart, msg, sizeof(msg), HAL_MAX_DELAY);
		}
		
}
// End Update FreeRTOS TASK -------------------------------------------------------------------------------------------
// ====================================================================================================================

// ====================================================================================================================
// Start Private Functions --------------------------------------------------------------------------------------------
static ETX_OTA_EX_ 
r_process_pack(void)
{

		const char *msg2;
	
		ETX_OTA_EX_ ret_val = ETX_OTA_EX_ERR;
		
		switch(g_ota_state)
		{
			
			case ETX_OTA_STATE_HEADER: 
			{
				ETX_OTA_HEADER_ *header = (ETX_OTA_HEADER_*)g_rx_buffer;
				
				if (header->packet_type == ETX_OTA_PACKET_TYPE_HEADER)
				{
					g_ota_fw_total_size = header->meta_data.package_size;
					g_ota_fw_crc        = header->meta_data.package_crc;
					
					HAL_StatusTypeDef status = r_clean_bank();
				
					if(status == HAL_OK)
					{
						g_ota_state = ETX_OTA_STATE_BULK_HEADER; 
						ret_val = ETX_OTA_EX_OK;
						
						const uint8_t msg[] = "HEADER_OK\n";
						HAL_UART_Transmit(p_uart, msg, sizeof(msg), HAL_MAX_DELAY);
					}
				}		
				
				break;
			}
			case ETX_OTA_STATE_BULK_HEADER:
			{
				ETX_OTA_BULK_HEADER_ *bulk_header = (ETX_OTA_BULK_HEADER_*)g_rx_buffer;
				
				if (bulk_header->packet_type == ETX_OTA_PACKET_TYPE_BULK_HEADER)
				{
					g_ota_bulk_crc = bulk_header->bulk_crc;
						
					g_ota_state = ETX_OTA_STATE_DATA; 
					ret_val = ETX_OTA_EX_OK;
					
					const uint8_t msg[] = "BULK_OK\n";
					HAL_UART_Transmit(p_uart, msg, sizeof(msg), HAL_MAX_DELAY);
				}	
				break;
			}
			
			case ETX_OTA_STATE_DATA: 
			{
				HAL_StatusTypeDef exe_state = HAL_ERROR;
				ETX_OTA_DATA_ *data_pack = (ETX_OTA_DATA_*)g_rx_buffer;
				
				if (data_pack->packet_type == ETX_OTA_PACKET_TYPE_DATA)
				{ 
					//Grabs the data and stores it in page_buffer. if completes buffer, inserts and saves the remnant
					exe_state = r_flash_process_data(data_pack->data, data_pack->data_len);
					
					const uint8_t msg[] = "DATA_OK\n";
					HAL_UART_Transmit(p_uart, msg, sizeof(msg), HAL_MAX_DELAY);
					
					if(exe_state == HAL_OK)
					{
						
						g_ota_state = ETX_OTA_STATE_BULK_HEADER;
						
						if(g_ota_fw_received_size >= g_ota_fw_total_size)
						{
							// Reset values for future updates.
							s_bank_a_index = APP_A_ADDRESS;
							s_page_offset = 0;
							memset_s(s_page_buffer, FLASH_PAGE_SIZE, 0xFF);
							
							g_ota_state = ETX_OTA_STATE_END;
						}
						
						if(g_ota_bulk_crc == s_pagecrc)
						{
							const uint8_t msg[] = "ACK";
							HAL_UART_Transmit(p_uart, msg, sizeof(msg), HAL_MAX_DELAY);
						} else 
						{
							const uint8_t msg[] = "NACK\n";
							HAL_UART_Transmit(p_uart, msg, sizeof(msg), HAL_MAX_DELAY);
						}
						
						ret_val = ETX_OTA_EX_OK;
					}
				}
				
				break;
			}
			
			case ETX_OTA_STATE_END: 
			{
				ETX_OTA_COMMAND_ *cmd = (ETX_OTA_COMMAND_*)g_rx_buffer;
				
				if (cmd->packet_type == ETX_OTA_PACKET_TYPE_CMD)
				{
					if(cmd->cmd == ETX_OTA_CMD_END)
					{
						//We do CRC of all the new Firmware stored in Bank
						s_flash_crc = r_calculate_flash_crc(g_ota_fw_received_size,APP_A_ADDRESS);
						
						g_ota_state = ETX_OTA_STATE_IDLE;
						if(s_flash_crc != g_ota_fw_crc)
						{
							ret_val = ETX_OTA_EX_ERR;
							
						} else 
						{
							ret_val = ETX_OTA_EX_OK;
							s_flag_flash_crc_ok = 1;
						}
					}
				}
				
				break;
			}
			default:{}
		}
		return ret_val;
}


// Start BANK FUNCTIONALITY -------------------------------------------------------------------------------------------
static HAL_StatusTypeDef 
r_clean_bank(void)
{
		HAL_StatusTypeDef status = HAL_ERROR;

		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_5);
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_9);

    // Desbloquea el control de la flash
    HAL_FLASH_Unlock();

    // Configura estructura de borrado
    FLASH_EraseInitTypeDef eraseInit;
    uint32_t pageError = 0;

    eraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
    eraseInit.Page = (APP_A_ADDRESS - REAL_FLASH_START) / FLASH_PAGE_SIZE;
    eraseInit.NbPages = (APP_MAX_SIZE / FLASH_PAGE_SIZE);
	
    // Borrado de páginas
		status = HAL_FLASHEx_Erase(&eraseInit, &pageError);

    // Bloquea de nuevo el control de la flash
    HAL_FLASH_Lock();

		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_5);
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_9);
		
    return status;
			
}
// End BANK FUNCTIONALITY ---------------------------------------------------------------------------------------------

// Start PAGE/DATA FUNCTIONALITY --------------------------------------------------------------------------------------
static HAL_StatusTypeDef 
r_flash_program_page(uint32_t address, uint8_t* data) 
{
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_5);
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_9);
	
		HAL_FLASH_Unlock();
    for (uint32_t i = 0; i < FLASH_PAGE_SIZE; i += 8) 
		{
        uint64_t data64 = 0;
        memcpy_s(&data64, sizeof(uint64_t), data + i, sizeof(uint64_t));
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address + i, data64);
    }
		HAL_FLASH_Lock();
		
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_5);
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_9);
		
		return HAL_OK;
}

static HAL_StatusTypeDef 
r_flash_process_data(uint8_t *data, uint16_t data_len)
{

		HAL_StatusTypeDef status = HAL_ERROR;
		const char *msg2;

		uint16_t space_left = FLASH_PAGE_SIZE - s_page_offset;
		uint16_t bytes_to_copy = data_len;

		if (bytes_to_copy > space_left)
				bytes_to_copy = space_left;

		// Copy data into page_buffer with correct offset
		memcpy_s(&s_page_buffer[s_page_offset], FLASH_PAGE_SIZE, data, bytes_to_copy);
		s_page_offset += bytes_to_copy;
		
		g_ota_fw_received_size += bytes_to_copy; // Add the inserted data size

		// Page_buffer full? we flash it into memory
		if (s_page_offset == FLASH_PAGE_SIZE) 
		{
				s_pagecrc = r_calculate_page_crc(s_page_buffer, FLASH_PAGE_SIZE);
				if(g_ota_bulk_crc == s_pagecrc)
				{
					status = r_flash_program_page(s_bank_a_index, s_page_buffer);
					s_bank_a_index += FLASH_PAGE_SIZE;
				}
				
				memset_s(s_page_buffer, FLASH_PAGE_SIZE, 0xFF); // Cleaning
				s_page_offset = 0;
				
		} else if(g_ota_fw_received_size >= g_ota_fw_total_size)
		{
			// We reach this section if the Page_buffer is not complete, and we already received all the data.
				s_pagecrc = r_calculate_page_crc(s_page_buffer, s_page_offset);
				if(g_ota_bulk_crc == s_pagecrc)
				{
					status = r_flash_program_last_data();
					s_bank_a_index += FLASH_PAGE_SIZE;
				}
				
				memset_s(s_page_buffer, FLASH_PAGE_SIZE, 0xFF); // Cleaning
				s_page_offset = 0;
		}
		return status;
}

static HAL_StatusTypeDef
r_flash_program_last_data()
{

		HAL_StatusTypeDef status = HAL_ERROR;
	
    // Complete buffer with 0xFF
    if (s_page_offset < FLASH_PAGE_SIZE)
		{
        memset_s(&s_page_buffer[s_page_offset], FLASH_PAGE_SIZE - s_page_offset, 0xFF);
    }

    // Flash Last Page
		status = r_flash_program_page(s_bank_a_index, s_page_buffer);
		return status;

}
// End PAGE/DATA FUNCTIONALITY ----------------------------------------------------------------------------------------
// ====================================================================================================================