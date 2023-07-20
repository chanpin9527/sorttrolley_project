#ifndef __MOTOR_DRIVER_H_
#define __MOTOR_DRIVER_H_

#include "main.h"


#define BRAKE_OFF		HAL_GPIO_WritePin(MOTOR_BRAKE_GPIO_Port,MOTOR_BRAKE_Pin,GPIO_PIN_SET)
#define BRAKE_ON		HAL_GPIO_WritePin(MOTOR_BRAKE_GPIO_Port,MOTOR_BRAKE_Pin,GPIO_PIN_RESET);

#define ON				1
#define OFF				0

#define	DIRECTION_FORE	HAL_GPIO_WritePin(MOTOR_DIRECTION_GPIO_Port,MOTOR_DIRECTION_Pin,GPIO_PIN_SET)
#define DIRECTION_REVE	HAL_GPIO_WritePin(MOTOR_DIRECTION_GPIO_Port,MOTOR_DIRECTION_Pin,GPIO_PIN_RESET);

#define DEC_FORE		1
#define	DEC_REVE		0


void init_motor(void);

void set_motor_speed(uint32_t speed);

void set_motor_speed_gear(uint8_t gear);

void set_motor_speedgeer_deriction(uint8_t gear, uint8_t deriction);

void motor_brake_on_off(uint8_t choice);

void motor_direction_change(uint8_t direction);

uint16_t read_motor_speed(void);

#endif
