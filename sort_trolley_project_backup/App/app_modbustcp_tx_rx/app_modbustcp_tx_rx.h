#ifndef __APP_MODBUSTCP_TX_RX_H_
#define __APP_MODBUSTCP_TX_RX_H_

#pragma pack(1)

#include "stm32f4xx_hal.h"

///***********************************add code***********************************/
#define REG_HOLDING_START     0x0001                // 保持寄存器起始地址
#define REG_HOLDING_NREGS     303                    // 保持寄存器数量

#define REG_INPUT_START       0x0000                // 输入寄存器起始地址
#define REG_INPUT_NREGS       2                    // 输入寄存器数量

#define REG_COILS_START       0x0000                // 线圈起始地址
#define REG_COILS_SIZE        2                    // 线圈数量

#define REG_DISCRETE_START    0x0000                // 开关寄存器起始地址
#define REG_DISCRETE_SIZE     2                    // 开关寄存器数量


/**************************需要获取的数据的寄存器地址***************************/
#define CAR_START_STOP_CMD_BUF_SIZE 2		
#define CAR_START_STOP_CMD_ADDR		0x0001	//小车启停寄存器地址

#define DRUM_PARABOLA_ARG_BUF_SIZE	10
#define DRUM_PARABOLA_ARG_ADDR		0x0010 	//甩货参数寄存器地址

#define CAR_SPEED_ARG_BUF_SIZE	6
#define CAR_SPEED_ARG_ADDR	0x0030			//小车速度参数寄存器地址

#define DRUM_ARG_BUF_SIZE	6
#define DRUM_ARG_ADDR		0x0033			//滚筒参数寄存器地址

/**************************需要设置的数据的寄存器地址***************************/
#define CAR_RUN_STATE_ARG_BUF_SIZE 	8
#define CAR_RUN_STATE_ARG_ADDR		0x0100	//小车运行状态寄存器地址

#define CAR_ALARM_DATA_BUF_SIZE		8
#define CAR_ALARM_DATA_ADDR			0x0104	//小车报警信息寄存器地址

#define TRAILER_PARCEL_BUF_SIZE		32
#define TRAILER_PARCEL_ADDR			0x0110	//拖车节有无包裹寄存器地址

#define SN_BUF_SIZE					32
#define SN_ADDR						0x0120	//拖车节对应的SN号寄存器地址



// 保持寄存器内容
extern uint16_t usRegHoldingBuf[REG_HOLDING_NREGS];
// 保持寄存器起始地址
extern uint16_t usRegHoldingStart;

/***************************防止警告*******************************/
extern uint16_t usRegInputBuf[REG_INPUT_NREGS];// 输入寄存器内容
extern uint16_t usRegInputStart;// 寄存器起始地址
extern uint8_t ucRegCoilsBuf[REG_COILS_SIZE];// 线圈状态
extern uint8_t ucRegDiscreteBuf[REG_DISCRETE_SIZE];// 开关状态

void init_read_data_sem(void);

void init_operate_sem(void);

void init_read_data_manager(void);

void set_read_data_sem(uint16_t write_addr);

void task_modbustcp_tx_rx(void const *arg);

#pragma pack()

#endif

