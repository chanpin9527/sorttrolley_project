#include "pid_driver.h"
#include "stdint.h"
#include "stdlib.h"
#include "math.h"


void inc_pid_control(Pid_arg_t *arg, const uint16_t target)
{
	float yn = 0;
	/*����ͼ������*/
	arg->e3 = arg->e2;
	arg->e2 = arg->e1;
	arg->e1 = target - arg->cur;
	yn = arg->I;
	/*PID��ʽ*/
	arg->I = arg->Ki*arg->e1;
	/*һ�׵�ͨ�˲���������)*/
	arg->I = arg->alpha*arg->I + (1-arg->alpha)*yn;
	arg->result += arg->Kp*(arg->e1-arg->e2) + arg->I + arg->Kd*(arg->e1 - 2*arg->e2 + arg->e3);
	/*��ֵ�޶�*/
	if(abs(arg->result) > arg->thrsod)
	{
		if(arg->result >= 0)
			arg->result = arg->thrsod;
		else
			arg->result = -arg->thrsod;
	}
}

