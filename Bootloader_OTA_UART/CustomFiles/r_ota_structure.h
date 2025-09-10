/**
 * @file r_ota_structure.h
 * @brief Definitions and structures for the ETX OTA (Over-The-Air) protocol.
 *
 * @author Manuel Martinez Leanes
 * @date 22/08/2025
 *
 * @details
 * This file defines macros, enums, and packed structures used to handle
 * OTA packets, commands, states, and update responses. It provides
 * all necessary formats for:
 * - Command packets
 * - Header and bulk header packets
 * - Data packets
 * - Response packets
 * 
 * These structures are packed to match the expected OTA protocol layout.
 */

#ifndef R_OTA_STRUCTURE_H
#define R_OTA_STRUCTURE_H

/** Start of Frame (SOF) for OTA packets */
#define ETX_OTA_SOF  					'$'
/** Carriage Return (CR) character */
#define ETX_OTA_SALTO_LINEA 	0x0D
/** Line Feed (LF) character */
#define ETX_OTA_FIN_LINEA			0x0A

/** Acknowledgment (ACK) code */
#define ETX_OTA_ACK  					0x00
/** Negative acknowledgment (NACK) code */
#define ETX_OTA_NACK 					0x01

/** Packet type sector inside protocol */
#define ETX_OTA_PACKET_TYPE_SECTOR	1	
/** Data sector inside protocol when Datapack */
#define ETX_OTA_DATA_SECTOR					4	

/** Maximum data size in an OTA packet */
#define ETX_OTA_DATA_MAX_SIZE			256
/** Data overhead in bytes in an OTA packet */
#define ETX_OTA_DATA_OVERHEAD			10 
/** Maximum total packet size (data + overhead) */
#define ETX_OTA_PACKET_MAX_SIZE ( ETX_OTA_DATA_MAX_SIZE + ETX_OTA_DATA_OVERHEAD )

//
/**
 * Exception codes
 */
typedef enum : uint8_t
{
  ETX_OTA_EX_OK       = 0,    // Success
  ETX_OTA_EX_ERR      = 1,    // Failure
}ETX_OTA_EX_;

/**
 * OTA process state
 */
typedef enum : uint8_t
{
  ETX_OTA_STATE_IDLE    		= 0,
  ETX_OTA_STATE_START   		= 1,
  ETX_OTA_STATE_HEADER  		= 2,
	ETX_OTA_STATE_BULK_HEADER = 3,
  ETX_OTA_STATE_DATA    		= 4,
  ETX_OTA_STATE_END    			= 5,
}ETX_OTA_STATE_;

/**
 * Packet type
 */
typedef enum : uint8_t
{
  ETX_OTA_PACKET_TYPE_CMD       		= 0,    // Command
  ETX_OTA_PACKET_TYPE_DATA      		= 1,    // Data
  ETX_OTA_PACKET_TYPE_HEADER    		= 2,    // Header
	ETX_OTA_PACKET_TYPE_BULK_HEADER   = 3,    // Header
  ETX_OTA_PACKET_TYPE_RESPONSE  		= 4,    // Response
}ETX_OTA_PACKET_TYPE_;

/**
 * OTA Commands
 */
typedef enum : uint8_t
{
  ETX_OTA_CMD_START = 0,    // OTA Start command
  ETX_OTA_CMD_END   = 1,    // OTA End command
  ETX_OTA_CMD_ABORT = 2,    // OTA Abort command
}ETX_OTA_CMD_;

//=================================================================================

/**
 * OTA meta info
 */
#pragma pack(push, 1)
typedef struct
{
  uint32_t package_size;
  uint32_t package_crc;
  uint32_t reserved1;
  uint32_t reserved2;
}__attribute__((packed)) meta_info;
#pragma pack(pop)

/**
 * OTA Command format
 *
 * ________________________________________
 * |     | Packet |     |     |     |     |
 * | SOF | Type   | Len | CMD | CRC | EOF |
 * |_____|________|_____|_____|_____|_____|
 *   1B      1B     2B    1B     4B    1B
 */
#pragma pack(push, 1)
typedef struct
{
  uint8_t   sof;
  ETX_OTA_PACKET_TYPE_   packet_type;
  uint16_t  data_len;
  uint8_t   cmd;
  uint32_t  crc;
  uint8_t   saltoLinea;
	uint8_t   finLinea;
}__attribute__((packed)) ETX_OTA_COMMAND_;
#pragma pack(pop)

/**
 * OTA Header format
 *
 * __________________________________________
 * |     | Packet |     | Header |     |     |
 * | SOF | Type   | Len |  Data  | CRC | EOF |
 * |_____|________|_____|________|_____|_____|
 *   1B      1B     2B     16B     4B    1B
 */
#pragma pack(push, 1)
typedef struct
{
  uint8_t     sof;
  ETX_OTA_PACKET_TYPE_     packet_type;
  uint16_t    data_len;
  meta_info   meta_data;
  uint32_t    crc;
  uint8_t   	saltoLinea;
	uint8_t   	finLinea;
}__attribute__((packed)) ETX_OTA_HEADER_;
#pragma pack(pop)

/**
 * OTA Bulk Header format
 *
 * __________________________________________
 * |     | Packet |     |  CRC   |     |     |
 * | SOF | Type   | Len |  BULK  | CRC | EOF |
 * |_____|________|_____|________|_____|_____|
 *   1B      1B     2B     16B     4B    1B
 */
#pragma pack(push, 1)
typedef struct
{
  uint8_t     sof;
  ETX_OTA_PACKET_TYPE_     packet_type;
  uint16_t    data_len;
  uint32_t    bulk_crc;
  uint32_t    crc;
  uint8_t   	saltoLinea;
	uint8_t   	finLinea;
}__attribute__((packed)) ETX_OTA_BULK_HEADER_;
#pragma pack(pop)

/**
 * OTA Data format
 *
 * __________________________________________
 * |     | Packet |     |        |     |     |
 * | SOF | Type   | Len |  Data  | CRC | EOF |
 * |_____|________|_____|________|_____|_____|
 *   1B      1B     2B    nBytes   4B    1B
 */
#pragma pack(push, 1)
typedef struct
{
  uint8_t     sof;
  ETX_OTA_PACKET_TYPE_     packet_type;
  uint16_t    data_len;
  //uint8_t     *data;
	uint8_t data[ETX_OTA_DATA_MAX_SIZE];
	uint32_t    crc;
}__attribute__((packed)) ETX_OTA_DATA_;
#pragma pack(pop)

/**
 * OTA Response format
 *
 * __________________________________________
 * |     | Packet |     |        |     |     |
 * | SOF | Type   | Len | Status | CRC | EOF |
 * |_____|________|_____|________|_____|_____|
 *   1B      1B     2B      1B     4B    2B
 */
#pragma pack(push, 1)
typedef struct
{
  uint8_t   sof;
  ETX_OTA_PACKET_TYPE_   packet_type;
  uint16_t  data_len;
  uint8_t   status;
  uint32_t  crc;
  uint8_t   saltoLinea;
	uint8_t   finLinea;
}__attribute__((packed)) ETX_OTA_RESP_;
#pragma pack(pop)

#endif /* R_OTA_STRUCTURE_H */