#include "motor_driver.h"
#include "dac.h"

#pragma pack(1) //�ڴ��ֽڶ���


#define DEBUG_MOTOR_PROTOCOL  // printf
#ifdef DEBUG_MOTOR_PROTOCOL
#define print_motor_log(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define print_motor_log(fmt, ...)
#endif

/**
 * @note      ��ʼ�����
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
 * @note      ���õ���ٶ�
 * @param     speed����ٶȣ�0-4095��
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
 * @note      ���õ���ٶȵ�λ
 * @param     gear�ٶȵ�λ��1-5�� 0ɲ����
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
	
	if(pre_dec != deriction) //����ı���
	{
		motor_brake_on_off(ON);
		set_motor_speed(0);
		motor_direction_change(deriction);
	}
	set_motor_speed_gear(gear);
	
	pre_dec = deriction;
}

/**
 * @note      ���õ��ɲ��
 * @param     choiceѡ���Ƿ�ɲ��BRAKE_ONɲ��BRAKE_OFF��ɲ��
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
 * @note      ���õ����ת����
 * @param     direction����DIRECTION_FORE��תDIRECTION_REVE��ת
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

