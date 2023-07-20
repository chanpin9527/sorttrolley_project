/*
 * File      : system_tick.h
 * This file is
 * notice:
 * Change Logs:
 * Date           Author       Notes
 * 2022-07-05     litao        create
 */
 
#ifndef __SYSTEM_TICK_H_
#define __SYSTEM_TICK_H_
#include "stdint.h"

/**
  * @note  获取系统时间-ms
  * @param  None
  * @return uint32_t systick  系统时间（s）
  */
uint64_t get_systick_time(void);

/**
  * @note  获取系统时间-ms
  * @param  None
  * @return uint32_t systick  系统时间（s）
  */
uint32_t get_systick_time_second(void);

#endif
