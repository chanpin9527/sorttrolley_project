#ifndef __PID_DRIVER_H
#define __PID_DRIVER_H

#include "stdint.h"

#pragma pack(1)

typedef struct
{
	void (*set_target)(int);
	int (*get_cur)(void);
//	PID����
	float alpha;			// һ�׵�ͨ�˲�ϵ��
	float Kp;
	float Ki;
	float Kd;
//	�������
	float I;				// �������ݴ�
	int e1;					// ���
	int e2;
	int e3;			
	int result;             // ������
	int thrsod;				// ��ֵ
	int cur;				// ʵ��ֵ
}Pid_arg_t;

void inc_pid_control(Pid_arg_t *arg, const uint16_t target);

#pragma pack()


#endif

