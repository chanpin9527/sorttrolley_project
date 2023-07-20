#include "main.h"
#include "led_driver.h"
#include "system_tick.h"
#include "cmsis_os.h"
#include "data_manager_data_type.h"

#pragma pack(1)

#define LED_PEROID		1000

static uint8_t fault_led[] = {[1] = 1, [2] = 4, [3] = 6, [4] = 8, [5] = 10, [6] = 12, [7] = 14};

uint64_t set_led_control(uint64_t tick_cur, uint64_t tick_pre)
{
	if(tick_cur - tick_pre >= LED_TOGGLE_INTERVAL) //200ms周期
	{
		tick_pre = tick_cur;
		HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin); //led灯
	}
	return tick_pre;
}

/*
*	车头没电
*	车尾没电
*/
uint64_t set_fault_led_state(uint8_t fault_mode, uint64_t tick_cur, uint64_t tick_pre)
{
	static uint8_t fault_count = 0;
	if(fault_count < fault_led[fault_mode])
	{
		if(tick_cur - tick_pre >= (LED_PEROID / (fault_led[fault_mode])))
		{
			fault_count++;
			tick_pre = tick_cur;
			HAL_GPIO_TogglePin(FAULT_LED_GPIO_Port, FAULT_LED_Pin);
		}
	}
	else
	{
		FAULT_LED_OFF;
		if(tick_cur - tick_pre >= LED_PEROID)
		{
			tick_pre = tick_cur;
			fault_count = 0;
			HAL_GPIO_TogglePin(FAULT_LED_GPIO_Port, FAULT_LED_Pin);
		}
	}
	return tick_pre;
}

void get_car_ip(Car_ip_t *car)
{
	car->area_id = 0;
	car->rail_id1 = HAL_GPIO_ReadPin(RAIL_2_GPIO_Port, RAIL_2_Pin) << 1 | HAL_GPIO_ReadPin(RAIL_1_GPIO_Port, RAIL_1_Pin);
	car->rail_id2 = HAL_GPIO_ReadPin(RAIL_0_GPIO_Port, RAIL_0_Pin) + 1;
	car->car_id = (HAL_GPIO_ReadPin(CARID2_GPIO_Port, CARID2_Pin) << 2) | \
					(HAL_GPIO_ReadPin(CARID1_GPIO_Port, CARID1_Pin) << 1) | \
					(HAL_GPIO_ReadPin(CARID0_GPIO_Port, CARID0_Pin));
}

#pragma pack()
