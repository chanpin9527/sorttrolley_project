#ifndef __APP_RS485_H_
#define __APP_RS485_H_

#pragma pack(1)

#include "stdint.h"

typedef struct
{
	uint32_t alarm_value;
}Drum_alarm_data_t;

#define DRUM_RUN_TIME_MIN			(500)
#define DRUM_RUN_TIME_MAX			(300)

void init_app485_data_manager(void);

void task_rs485_tx(void const *arg);

#pragma pack()

#endif

