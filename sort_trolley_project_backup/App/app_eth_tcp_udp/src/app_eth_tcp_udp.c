#include "app_eth_tcp_udp.h"
#include "data_manager_data_type.h"
#include "data_manager.h"
#include "cmsis_os.h"
#include "stdio.h"
#include "string.h"
#include "lidar_driver.h"
#include "eth_tcp_driver.h"
#include "eth_udp_driver.h"
#include "system_tick.h"

#define DEBUG_ETH_CONTROL  // printf
#ifdef DEBUG_ETH_CONTROL
#define print_eth_log(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define print_eth_log(fmt, ...)
#endif


//获取的数据(原始雷达数据包)
static Lidar_inital_data_t s_lidar_inital_data = {0};

//解析后的雷达数据
static Lidar_data_t s_lidar_valid_data_temp[LIDAR_VALID_DATA_SIZE] = {0};
static uint8_t s_lidar_valid_data_num = 0;

//需要写的数据
static Lidar_strike_signal_t s_lidar_strike_signal = {0};
static Data_manager_dev *s_lidar_strike_signal_dev = NULL;
static uint8_t s_lidar_strike_flag = 0;
static uint8_t lidar_data_index = 0;

SemaphoreHandle_t lidar_data_semp = NULL;
TimerHandle_t	lidar_timeout_timer_Handle;
#define LIDAR_INITAL_DATA_Q_NUM	10
QueueHandle_t lidar_init_data_msg_mq;

#define REALY_BARRIER_RSSI	255


static void handle_lidar_data(Lidar_data_t *lidar_data, uint8_t data_size)
{
	if(lidar_data == NULL || data_size == 0)
	{
		s_lidar_valid_data_num = 0;
		return ;
	}
	s_lidar_valid_data_num = data_size;
	memcpy(s_lidar_valid_data_temp, lidar_data, data_size * sizeof(Lidar_data_t));
}


static int8_t get_lidar_lidar_signal(Lidar_data_t *lidar_data, uint8_t data_len)
{
	uint8_t i = 0;
	uint8_t lidar_signal = NO_STRIKE_SIGNAL;
	
	if(lidar_data == NULL)
		return -1;
	if(data_len == 0)
		return lidar_signal;
	
	for(i = 0; i < data_len; i++)
	{
		if((lidar_data[i].rssi >= REALY_BARRIER_RSSI) && (lidar_data[i].dist >= LIDAR_DATA_MIN && lidar_data[i].dist <= LIDAR_DATA_MAX))
		{
			lidar_signal = STRIKE_SIGNAL;
			break;
		}
	}
	
	return lidar_signal;
}

static void lidar_rx_timeout_timer_cb(TimerHandle_t xTimer)
{
		//无雷达数据
	print_eth_log("\r\nlidar sensor or eth is failed!!!\r\n");
	s_lidar_strike_signal.strike_signal = STRIKE_SIGNAL;
	if(set_data_manager_data(s_lidar_strike_signal_dev, &s_lidar_strike_signal, sizeof(Lidar_strike_signal_t))!= sizeof(Lidar_strike_signal_t))
	{
		print_eth_log("data manager set s_lidar_data_dev error\n");
	}
	xSemaphoreGive(lidar_data_semp);//同步雷达数据传输
}

void task_eth_tcp_udp(void const *arg)
{
	BaseType_t err;
	int8_t lidar_strike_signal= NO_STRIKE_SIGNAL;
	uint32_t count = 0;
	uint32_t tick_cur = 0, tick_alive = 0;
	
	lidar_init_data_msg_mq = xQueueCreate(LIDAR_INITAL_DATA_Q_NUM, sizeof(Lidar_inital_data_t)); //创建雷达消息队列
	lidar_data_semp = xSemaphoreCreateCounting(1, 0);
	s_lidar_strike_signal_dev = create_data_manager("lidar_strike_signal", sizeof(Lidar_strike_signal_t), 0);
	lidar_timeout_timer_Handle = xTimerCreate("lidar_no_data", 5000, pdFALSE, (void *)0, lidar_rx_timeout_timer_cb); //5s创建雷达数据定时器
	xTimerStop(lidar_timeout_timer_Handle, 0);
	
	osDelay(2200); //等待底层网卡初始化完成
	udp_server_init(); //初始化udp服务端
	
//	OTA_tcp_server_init(); 
	while(1)
	{
		/*
		处理激光雷达防撞
		*/
		tick_cur = get_systick_time();
		if(tick_cur - tick_alive >= (TASK_ALIVE_PRINT_PERIOD + 300))
		{
			tick_alive = tick_cur;
			print_eth_log("\r\ntask eth lidar is aliving!!!\r\n");
		}
		err = xQueueReceive(lidar_init_data_msg_mq, &s_lidar_inital_data, 1000);//获取雷达原始数据
		if(err == pdTRUE)
		{
			analysis_lidar_data(s_lidar_inital_data.lidar_data, handle_lidar_data); //解析雷达数据
			lidar_strike_signal = get_lidar_lidar_signal(s_lidar_valid_data_temp, s_lidar_valid_data_num);
			if(s_lidar_inital_data.frame_head == LEFT_LIDAR_FRAME_HEAD)
			{
				if(lidar_strike_signal == 0)
				{
					s_lidar_strike_flag &= ~(0x01 << LEFT_LIDAR_DATA_INDEX);
				}
				else
				{
					s_lidar_strike_flag |= 0x01 << LEFT_LIDAR_DATA_INDEX;
				}
				if(s_lidar_strike_flag != 0)
				{
					lidar_data_index = RIGHT_LIDAR_DATA_INDEX;
				}
				else
				{
					lidar_data_index = LEFT_LIDAR_DATA_INDEX;
				}
			}
			else if(s_lidar_inital_data.frame_head == MIDDLE_LIDAR_FRAME_HEAD)
			{
				if(lidar_strike_signal == 0)
				{
					s_lidar_strike_flag &= ~(0x01 << MIDDLE_LIDAR_DATA_INDEX);
				}
				else
				{
					s_lidar_strike_flag |= 0x01 << MIDDLE_LIDAR_DATA_INDEX;
				}
				if(s_lidar_strike_flag != 0)
				{
					lidar_data_index = RIGHT_LIDAR_DATA_INDEX;
				}
				else
				{
					lidar_data_index = MIDDLE_LIDAR_DATA_INDEX;
				}
			}
			else if(s_lidar_inital_data.frame_head == RIGHT_LIDAR_FRAME_HEAD)
			{
				if(lidar_strike_signal == 0)
				{
					s_lidar_strike_flag &= ~(0x01 << RIGHT_LIDAR_DATA_INDEX);
				}
				else
				{
					s_lidar_strike_flag |= 0x01 << RIGHT_LIDAR_DATA_INDEX;
				}
				lidar_data_index = RIGHT_LIDAR_DATA_INDEX;
			}
			
			if(lidar_data_index == RIGHT_LIDAR_DATA_INDEX)
			{
				s_lidar_strike_signal.strike_signal = (!!(s_lidar_strike_flag));
				if(set_data_manager_data(s_lidar_strike_signal_dev, &s_lidar_strike_signal, sizeof(Lidar_strike_signal_t))!= sizeof(Lidar_strike_signal_t))
				{
					print_eth_log("data manager set s_lidar_data_dev error\n");
				}
				xSemaphoreGive(lidar_data_semp);//同步雷达数据传输
			}
			xTimerStart(lidar_timeout_timer_Handle, 0);
			xTimerStop(lidar_timeout_timer_Handle, 0);
		}
		/*
		处理OTA远程升级
		*/
	}
}


