#ifndef __RS485_DRIVER_H_
#define __RS485_DRIVER_H_

#include "main.h"

#pragma pack(1)

#define RS485_TX_EN		HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_SET)
#define RS485_RX_EN		HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_RESET)

#define RUN_FRAME_HEAD	0x8A
#define RESERVE			0x00
#define ACK_FRAME_HEAD	0x99

#define RUN_SPEED	255
#define RUN_ACC		255
#define BACK_SPEED	20
#define BACK_ACC	50

typedef enum
{
	SINGLE_FRAME_ACK = 0,
	SINGLE_FRAME_NO_ACK,
	DOUBLE_FRAME_ACK,
	DOUBLE_FRAME_NO_ACK,
}Run_type;

typedef enum
{
	FOREWORD = 0,
	BACKWORD,
	MAXDEC,
}Run_dec;

enum
{
	LEFT_DEC = 0,
	RIGHT_DEC,
};

typedef enum
{
	TIME_CTL = 0,
	LOCTION_CTL,
}Control_mode;


typedef union
{
	struct
	{
		uint8_t over_temp_fault:	1;
		uint8_t over_curr_fault:	1;
		uint8_t	encoder_fault:		1;
		uint8_t no_arg_fault:		1;
		uint8_t	no_run_fault:		1;
		uint8_t	locked_fault:		1;
		uint8_t	overload_fault:		1;
		uint8_t	over_vol_fault:		1;
	}Bit_fault;
	uint8_t	fault_value;
}Fault_type;


void drum_run_forword_backword(uint8_t slaveID, uint8_t direction, uint8_t turn_dec);


#pragma pack()

#endif
