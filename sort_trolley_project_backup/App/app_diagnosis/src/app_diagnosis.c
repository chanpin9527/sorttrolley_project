#include "app_diagnosis.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "string.h"
#include "cmsis_os.h"
#include "dp83848.h"
#include "data_manager_data_type.h"
#include "data_manager.h"
#include "system_tick.h"
#include "rs485_driver.h"
#include "easyflash.h"
#include "app_modbustcp_tx_rx.h"
#include "math.h"
#include "stdlib.h"
#include "iwdg_driver.h"


#pragma pack(1)

#define DEBUG_DIAGNOSIS  // printf
#ifdef DEBUG_DIAGNOSIS
#define print_diagnosis_log(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define print_diagnosis_log(fmt, ...)
#endif


void task_diagnosis(void const *arg)
{
	char tasklist_buffer[512] = {0};
	uint32_t tick_cur = 0, tick_task_list = 0, tick_alive = 0;
	print_diagnosis_log("\r\nenter diagnosis task!!!\r\n");
	
	while(1)
	{
		iwdg_feeddog();
		tick_cur = get_systick_time();
		if(tick_cur - tick_alive >= (TASK_ALIVE_PRINT_PERIOD + 600))
		{
			tick_alive = tick_cur;
			print_diagnosis_log("\r\ntask diagnosis is aliving!!!\r\n");
		}
		if(tick_cur - tick_task_list >= (TASK_STATE_PRINT_PERIOD))
		{
			tick_task_list = tick_cur;
			vTaskList((char *) &tasklist_buffer);
			print_diagnosis_log("\r\nTask_name      		     state  priority   stack   Id\r\n");
			print_diagnosis_log("\r\n%s\r\n", tasklist_buffer);
		}
		osDelay(200);
	}
}


#pragma pack()

