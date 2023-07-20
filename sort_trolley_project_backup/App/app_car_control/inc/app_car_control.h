#ifndef __APP_CAR_CONTROL_H_
#define __APP_CAR_CONTROL_H_


#pragma pack(1)

#ifndef MIN
#define MIN(x,y)	((x) < (y) ? (x) : (y))
#endif

#ifndef MAX
#define MAX(x,y)	((x) > (y) ? (x) : (y))
#endif


enum
{
	CAR_HEAD_BATTERY_VOL 	= 0,
	DRUM_BATTERY_VOL 		= 1,
	BATTERY_VOL_MAX_INDEX 	= 2,
};

enum
{
	CAR_STATE_ALARM 		= 1,
	CAR_HEAD_BATTERY_ALARM 	= 2, 
	DRUM_BATTERY_ALARM 		= 4,
};

void handle_tim_irq(void);

void init_set_data_manager(void);

void task_car_control(void const *arg);

#pragma pack()

#endif


