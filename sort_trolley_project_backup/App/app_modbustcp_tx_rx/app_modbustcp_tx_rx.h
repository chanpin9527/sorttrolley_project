#ifndef __APP_MODBUSTCP_TX_RX_H_
#define __APP_MODBUSTCP_TX_RX_H_

#pragma pack(1)

#include "stm32f4xx_hal.h"

///***********************************add code***********************************/
#define REG_HOLDING_START     0x0001                // ���ּĴ�����ʼ��ַ
#define REG_HOLDING_NREGS     303                    // ���ּĴ�������

#define REG_INPUT_START       0x0000                // ����Ĵ�����ʼ��ַ
#define REG_INPUT_NREGS       2                    // ����Ĵ�������

#define REG_COILS_START       0x0000                // ��Ȧ��ʼ��ַ
#define REG_COILS_SIZE        2                    // ��Ȧ����

#define REG_DISCRETE_START    0x0000                // ���ؼĴ�����ʼ��ַ
#define REG_DISCRETE_SIZE     2                    // ���ؼĴ�������


/**************************��Ҫ��ȡ�����ݵļĴ�����ַ***************************/
#define CAR_START_STOP_CMD_BUF_SIZE 2		
#define CAR_START_STOP_CMD_ADDR		0x0001	//С����ͣ�Ĵ�����ַ

#define DRUM_PARABOLA_ARG_BUF_SIZE	10
#define DRUM_PARABOLA_ARG_ADDR		0x0010 	//˦�������Ĵ�����ַ

#define CAR_SPEED_ARG_BUF_SIZE	6
#define CAR_SPEED_ARG_ADDR	0x0030			//С���ٶȲ����Ĵ�����ַ

#define DRUM_ARG_BUF_SIZE	6
#define DRUM_ARG_ADDR		0x0033			//��Ͳ�����Ĵ�����ַ

/**************************��Ҫ���õ����ݵļĴ�����ַ***************************/
#define CAR_RUN_STATE_ARG_BUF_SIZE 	8
#define CAR_RUN_STATE_ARG_ADDR		0x0100	//С������״̬�Ĵ�����ַ

#define CAR_ALARM_DATA_BUF_SIZE		8
#define CAR_ALARM_DATA_ADDR			0x0104	//С��������Ϣ�Ĵ�����ַ

#define TRAILER_PARCEL_BUF_SIZE		32
#define TRAILER_PARCEL_ADDR			0x0110	//�ϳ������ް����Ĵ�����ַ

#define SN_BUF_SIZE					32
#define SN_ADDR						0x0120	//�ϳ��ڶ�Ӧ��SN�żĴ�����ַ



// ���ּĴ�������
extern uint16_t usRegHoldingBuf[REG_HOLDING_NREGS];
// ���ּĴ�����ʼ��ַ
extern uint16_t usRegHoldingStart;

/***************************��ֹ����*******************************/
extern uint16_t usRegInputBuf[REG_INPUT_NREGS];// ����Ĵ�������
extern uint16_t usRegInputStart;// �Ĵ�����ʼ��ַ
extern uint8_t ucRegCoilsBuf[REG_COILS_SIZE];// ��Ȧ״̬
extern uint8_t ucRegDiscreteBuf[REG_DISCRETE_SIZE];// ����״̬

void init_read_data_sem(void);

void init_operate_sem(void);

void init_read_data_manager(void);

void set_read_data_sem(uint16_t write_addr);

void task_modbustcp_tx_rx(void const *arg);

#pragma pack()

#endif

