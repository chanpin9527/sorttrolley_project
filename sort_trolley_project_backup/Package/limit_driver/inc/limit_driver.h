#ifndef __LIMIT_DRIVER_H_
#define __LIMIT_DRIVER_H_

#include "gpio.h"
#include "main.h"

#define LIMIT_SLAVE_STATE(n)	HAL_GPIO_ReadPin(LIMIT##n##_GPIO_Port, LIMIT##n##_Pin)

#define LIMIT_STATE_READ(SLAVE)	LIMIT_SLAVE_STATE(SLAVE)

uint8_t read_limit_state(uint8_t slave);

#endif

