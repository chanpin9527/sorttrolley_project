#ifndef __LED_DRIVER_H_
#define __LED_DRIVER_H_

#include "stdint.h"
#include "main.h"
#include "data_manager_data_type.h"

#define LED_TOGGLE_INTERVAL		100	

uint64_t set_led_control(uint64_t tick_cur,uint64_t tick_pre);

uint64_t set_fault_led_state(uint8_t fault_mode, uint64_t tick_cur, uint64_t tick_pre);

void get_car_ip(Car_ip_t *car);

#define RUN_LED_ON 		    HAL_GPIO_WritePin(RUN_LED_GPIO_Port, RUN_LED_Pin, GPIO_PIN_SET)
#define RUN_LED_OFF 		HAL_GPIO_WritePin(RUN_LED_GPIO_Port, RUN_LED_Pin, GPIO_PIN_RESET)
#define FAULT_LED_ON	    HAL_GPIO_WritePin(FAULT_LED_GPIO_Port, FAULT_LED_Pin, GPIO_PIN_SET)
#define FAULT_LED_OFF	    HAL_GPIO_WritePin(FAULT_LED_GPIO_Port, FAULT_LED_Pin, GPIO_PIN_RESET)



#endif

