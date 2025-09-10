#ifndef R_UART_CALLBACK_H
#define R_UART_CALLBACK_H

#include "stdint.h"
#include "stm32wlxx_hal.h"
#include "r_task_update.h"
#include "r_flags.h"

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);

#endif //R_UART_CALLBACK_H