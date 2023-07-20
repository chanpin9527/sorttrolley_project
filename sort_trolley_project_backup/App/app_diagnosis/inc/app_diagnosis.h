#ifndef __APP_DIAGNOSIS_H_
#define __APP_DIAGNOSIS_H_

#include "stdint.h"

#pragma pack(1)

typedef struct
{
	uint8_t eth_link;
}Eth_link_state_t;

enum
{
	ETH_LINK = 0,
	ETH_NO_LINK = 1
};

typedef enum
{
	PORRST = 1, //�ϵ�������λ
	PINRST, 	//�ⲿ������λ
	SFTRST, 	//�����λ
	IWDGRST, 	//���Ź���λ
}System_reset_mode_t;

void task_diagnosis(void const *arg);


#pragma pack()

#endif


