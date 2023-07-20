#ifndef __RS485_MIDDLE_H_
#define __RS485_MIDDLE_H_

#include "stm32f4xx_hal.h"

#define RS485_TX_EN		HAL_GPIO_WritePin(RS485_RE_DE_GPIO_Port, RS485_RE_DE_Pin, GPIO_PIN_SET)
#define RS485_RX_EN		HAL_GPIO_WritePin(RS485_RE_DE_GPIO_Port, RS485_RE_DE_Pin, GPIO_PIN_RESET)

uint32_t rs485_send_data(const char *buffer, uint16_t length);

uint32_t rs485_read_data(uint8_t *buffer, uint16_t length);

int32_t modbus_driver_init(void *dev_mut);

void task_rs485(void const *arg);


#endif

