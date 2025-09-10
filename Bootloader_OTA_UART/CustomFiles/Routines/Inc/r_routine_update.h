/**
 * @file r_task_update.h
 * @brief FreeRTOS OTA firmware update task interface.
 *
 * @author Manuel Martinez Leanes
 * @date 22/08/2025
 *
 */

#ifndef R_TASK_UPDATE_H
#define R_TASK_UPDATE_H

#include "safe_mem_lib.h"
#include "safe_str_lib.h"
//#include "usart.h"

#include "r_crc.h"
#include "r_ota_structure.h"
#include "r_eeprom_structure.h"

#include "r_flash_addresses.h"

/**
 * @brief Raw receive buffer for incoming OTA packets.
 */
extern volatile uint8_t g_rx_buffer[ETX_OTA_PACKET_MAX_SIZE];

/**
 * @brief Total firmware size (in bytes) expected during OTA.
 */
extern volatile uint32_t g_ota_fw_total_size;

/**
 * @brief Expected CRC32 checksum of the incoming firmware.
 */
extern volatile uint32_t g_ota_fw_crc;

/**
 * @brief Total number of firmware bytes received so far.
 */
extern volatile uint32_t g_ota_fw_received_size;

/**
 * @brief OTA state machine current state.
 */
extern volatile ETX_OTA_STATE_ g_ota_state;

/**
 * @brief OTA update reception TASK.
 * Waits for FLAG_RECEIVE_UPDATE and processes incoming firmware packets.
 * @param argument Not used.
 */
void r_receive_update();

#endif // R_TASK_UPDATE_H