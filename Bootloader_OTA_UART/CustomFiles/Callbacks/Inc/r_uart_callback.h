#ifndef R_UART_CALLBACK_H
#define R_UART_CALLBACK_H

#include "stdint.h"
#include "stm32wlxx_hal.h"
#include "r_startup.h"
#include "r_routine_update.h"

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);

#endif //R_UART_CALLBACK_H