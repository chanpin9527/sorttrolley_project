/*
 * File      : system_tick.c
 * This file is
 * notice:
 * Change Logs:
 * Date           Author       Notes
 * 2022-07-05     litao        create
 */
 #include "main.h"
#include "cmsis_os.h"
#include "system_tick.h"

#pragma pack(1) //内存字节对齐

static uint64_t s_systick_pre = 0;
static uint64_t systick_overflow_count = 0;

/**
  * @note  获取系统时间-ms
  * @param  None
  * @return uint32_t systick  系统时间（s）
  */
uint64_t get_systick_time(void)
{
	uint64_t systick_cur = 0;
	uint64_t systick_res = 0;
	systick_cur = osKernelSysTick();
    if(systick_cur < s_systick_pre)
    {
    	systick_overflow_count++;
    }
    s_systick_pre = systick_cur;
    systick_res = systick_cur + (systick_overflow_count << 32);
	return systick_res;
}

/**
  * @note  获取系统时间-ms
  * @param  None
  * @return uint32_t systick  系统时间（s）
  */
uint32_t get_systick_time_second(void)
{
	uint64_t systick = 0;
    systick = get_systick_time()/1000;
	return systick;
}

#pragma pack() //内存字节对齐


