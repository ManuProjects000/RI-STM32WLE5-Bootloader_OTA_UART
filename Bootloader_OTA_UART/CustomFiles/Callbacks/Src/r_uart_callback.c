#include "r_uart_callback.h"


/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/**
 * @brief Current write index for @ref uart_buffer.
 *
 * Incremented with each received byte. Resets once a packet is processed.
 */
static volatile uint16_t s_uart_index = 0;

/**
 * @brief Flag indicating that a complete OTA packet has been received.
 *
 * Set to 1 when a packet is ready for processing by the update task.
 */
volatile uint8_t s_packet_ready = 0;

/**
 * @brief UART reception buffer for OTA packets.
 *
 * Stores incoming bytes until a complete OTA packet is received.
 */
static uint8_t s_uart_buffer[ETX_OTA_PACKET_MAX_SIZE] = {0};

/**
 * @brief Temporary storage for the most recently received UART byte.
 */
volatile uint8_t s_rx_byte = 0;

/**
 * @brief Previous byte received over UART.
 *
 * Used to detect packet delimiters (end-of-frame = 0A0D).
 */
static uint8_t s_last_byte = 0;

/**
 * @brief Length of the payload currently being processed.
 */
static uint16_t s_len_payload = 0;

/* USER CODE END PTD */

void 
HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) 
{
    uint8_t byte = s_rx_byte;  // byte recibido por UART
		
    s_uart_buffer[s_uart_index] = byte;
		s_uart_index++;
		
		if(s_uart_index == 4)
		{
			s_len_payload = s_uart_buffer[2] | (s_uart_buffer[3] << 8);
			s_len_payload += 4;
			
		} else 
		{
			if(s_len_payload > 0)
			{
				s_len_payload--;
				
			} else	
			{
				if ((s_last_byte == ETX_OTA_SALTO_LINEA) && (byte == ETX_OTA_FIN_LINEA)) 
				{  // fin de paquete
						s_packet_ready = 1;
					
				}
			}
		}
		
		s_last_byte = byte; // actualizar último byte recibido
		
		//HAL_UART_Transmit(&huart2, &byte, 1, HAL_MAX_DELAY);
		
		if(s_packet_ready)
		{
			memcpy_s((void*)g_rx_buffer, ETX_OTA_PACKET_MAX_SIZE,(void*)s_uart_buffer, s_uart_index);

			s_uart_index = 0;
			//s_packet_ready = 0;
		}

    // reactivar la interrupción para siguiente byte
    HAL_UART_Receive_IT(huart, (uint8_t *)&s_rx_byte, 1);
}