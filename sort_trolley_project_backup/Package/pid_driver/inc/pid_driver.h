#ifndef __PID_DRIVER_H
#define __PID_DRIVER_H

#include "stdint.h"

#pragma pack(1)

typedef struct
{
	void (*set_target)(int);
	int (*get_cur)(void);
//	PID参数
	float alpha;			// 一阶低通滤波系数
	float Kp;
	float Ki;
	float Kd;
//	计算相关
	float I;				// 积分项暂存
	int e1;					// 误差
	int e2;
	int e3;			
	int result;             // 计算结果
	int thrsod;				// 阈值
	int cur;				// 实际值
}Pid_arg_t;

void inc_pid_control(Pid_arg_t *arg, const uint16_t target);

#pragma pack()


#endif

