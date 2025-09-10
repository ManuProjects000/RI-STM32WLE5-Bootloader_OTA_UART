/**
 * @file r_crc.h
 * @brief CRC-32 calculation utilities for flash memory and OTA firmware.
 *
 * @author Manuel Martinez Leanes
 * @date 22/08/2025
 *
 */


#ifndef R_CRC_H
#define R_CRC_H

#include "main.h"
#include "stdint.h"

#include "r_flash_addresses.h"

/**
 * @brief Calculate CRC over firmware data stored in flash memory.
 *
 * This function computes a CRC32 value for a firmware image stored in flash.
 * The algorithm processes the data word by word (4 bytes), and handles any
 * remaining bytes at the end.
 *
 * @param ota_fw_received_size   Size of the firmware image in bytes.
 * @param address                Starting address of the firmware in flash.
 * @return Computed CRC32 value.
 */
uint32_t r_calculate_flash_crc(uint32_t ota_fw_received_size, uint32_t address);

/**
 * @brief Calculate CRC over a 32-bit word (16 bytes of data).
 *
 * This function computes a CRC32 value over 16 bytes of input data,
 * treating it as four 32-bit words.
 *
 * @param data   Pointer to the input data (minimum 16 bytes).
 * @return Computed CRC32 value.
 */
uint32_t r_calculate_word_crc(uint8_t *data);

/**
 * @brief Calculate CRC over a page of data.
 *
 * This function computes a CRC32 value over a memory page. The page length
 * may not necessarily be a multiple of 4, so any remaining bytes are ignored.
 *
 * @param data_page   Pointer to the page data.
 * @param length      Size of the page in bytes.
 * @return Computed CRC32 value.
 */
uint32_t r_calculate_page_crc(uint8_t *data_page, size_t length);

#endif /* R_CRC_H */