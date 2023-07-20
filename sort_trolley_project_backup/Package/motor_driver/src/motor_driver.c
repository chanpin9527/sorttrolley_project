#include "motor_driver.h"
#include "dac.h"

#pragma pack(1) //内存字节对齐


#define DEBUG_MOTOR_PROTOCOL  // printf
#ifdef DEBUG_MOTOR_PROTOCOL
#define print_motor_log(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define print_motor_log(fmt, ...)
#endif

/**
 * @note      初始化电机
 * @param     None
 * @return    None
 */
void init_motor(void)
{
	HAL_DAC_Start(&hdac, DAC_CHANNEL_1);
	HAL_DAC_Start(&hdac, DAC_CHANNEL_2);
	
	motor_brake_on_off(ON);
	motor_direction_change(DEC_FORE);
}

/**
 * @note      设置电机速度
 * @param     speed电机速度（0-4095）
 * @return    None
 */
void set_motor_speed(uint32_t speed)
{
	if(speed > 4095)
		speed = 4095;
	
	motor_brake_on_off(OFF);
	HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, speed);
	HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, speed);
}

/**
 * @note      设置电机速度档位
 * @param     gear速度档位（1-5， 0刹车）
 * @return    None
 */
void set_motor_speed_gear(uint8_t gear)
{
	uint32_t speed = 0;
	
	switch(gear)
	{
		case 1: speed = 1950; break; //1100
		case 2: speed = 4095; break; // 1638
		case 3: speed = 2457; break; //2457
		case 4: speed = 3276; break; //3276
		case 5: speed = 4095; break; //4095
		default : break;
	}
	
	set_motor_speed(speed);
}

void set_motor_speedgeer_deriction(uint8_t gear, uint8_t deriction)
{
	static uint8_t pre_dec = 0;
	
	if(gear < 0 || gear > 5 || (deriction != DEC_FORE && deriction != DEC_REVE))
		return ;
	
	if(pre_dec != deriction) //方向改变了
	{
		motor_brake_on_off(ON);
		set_motor_speed(0);
		motor_direction_change(deriction);
	}
	set_motor_speed_gear(gear);
	
	pre_dec = deriction;
}

/**
 * @note      设置电机刹车
 * @param     choice选择是否刹车BRAKE_ON刹车BRAKE_OFF不刹车
 * @return    None
 */
void motor_brake_on_off(uint8_t choice)
{
	if(choice == ON)
	{
		BRAKE_ON;
		HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 4095);
		HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, 4095);
	}
	else
	{
		BRAKE_OFF;
	}
}

/**
 * @note      设置电机运转方向
 * @param     direction方向DIRECTION_FORE正转DIRECTION_REVE反转
 * @return    None
 */
void motor_direction_change(uint8_t direction)
{
	direction ? DIRECTION_FORE : DIRECTION_REVE;
}

uint16_t read_motor_speed(void)
{
	return ((HAL_DAC_GetValue(&hdac, DAC_CHANNEL_2)+HAL_DAC_GetValue(&hdac, DAC_CHANNEL_2))/2);
}

#pragma pack()

