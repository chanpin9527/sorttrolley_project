#include "app_mqtt_tx.h"
#include "cmsis_os.h"
#include "mqtt_driver.h"
#include "mqtt_protocol.h"
#include "data_manager.h"
#include "data_manager_data_type.h"
#include "string.h"
#include "stdio.h"
#include "system_tick.h"
#include "rs485_driver.h"
#include "convert_endian.h"
#include "flash_parameter.h"


#define DEBUG_MQTT_TX  // printf
#ifdef DEBUG_MQTT_TX
#define print_mqtt_tx_log(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define print_mqtt_tx_log(fmt, ...)
#endif

#define MQTT_RECONNECT_PERIOD			(1800)
#define MQTT_CONNECT_TIMEOUT_TIME		(20000)
#define MQTT_CONNECT_TIMEOUT_COUNT		(3)
#define MQTT_SEND_BUFF_SIZE				512
#define SN_SIZE							20

static uint8_t s_mqtt_connect_count = 0;

static uint16_t send_get_operating_para_mag(void *arg,uint8_t *);
static uint16_t send_report_railtime_data_msg(void *arg,uint8_t *);
static uint16_t send_report_package_infromation_msg(void *arg,uint8_t *);
static uint16_t send_report_alarm_data_msg(void *arg,uint8_t *);

static Record_operating_data_t s_record_data = {0};
static Get_operating_para_t s_request_para = {0};
static Report_realtime_operating_data_t s_car_run_data = {0};
static Set_package_informtion_t s_package_information[SLAVE_END] = {0};
static Report_alarm_data_t s_car_alarm_data = {0};
static Report_alarm_data_t cmp_car_alarm_data = {0};
static uint8_t package_data[MQTT_SEND_BUFF_SIZE] = {0};

static Data_manager_dev *s_car_run_data_dev = NULL;
static Data_manager_dev *s_car_alarm_dev = NULL;
static Data_manager_dev* s_package_info_dev = NULL;
static Data_manager_dev* s_record_data_dev = NULL;

static uint8_t s_mqtt_send_data_buf[MQTT_SEND_BUFF_SIZE] = {0};


TimerHandle_t mqtt_tx_timeout_timer_Handle;
TimerHandle_t mqtt_connect_timeout_timer_Handle;


typedef struct
{
	uint32_t tick_pre;
	uint32_t interval;
	uint32_t data_type;
	uint16_t (*send_data_handle)(void *,uint8_t *);
}Mqtt_send_handle_t;

Mqtt_send_handle_t s_mqtt_send_handle_struct[] = {
	{0, 0, GET_OPERATING_PARA, send_get_operating_para_mag},
	{0, 100, REPORT_REALTIME_DATA, send_report_railtime_data_msg},
	{0, 1000, REPORT_PACKAGE_INFORMATION, send_report_package_infromation_msg},
	{0, 10, REPORT_ALARM_DATA, send_report_alarm_data_msg},
};

static uint16_t send_get_operating_para_mag(void *arg,uint8_t *send_buf)
{	
    uint16_t data_len = 0;
    Get_operating_para_t *operating_para = (Get_operating_para_t *)arg;
	convert_u32_to_big_endian((uint32_t *)operating_para);
	data_len = mqtt_package_data(GET_OPERATING_PARA, (uint8_t *)operating_para, sizeof(Get_operating_para_t), send_buf); 
    return data_len;
}

static uint16_t send_report_railtime_data_msg(void *arg,uint8_t *send_buf)
{
    uint16_t data_len = 0;
    Report_realtime_operating_data_t *report_data = (Report_realtime_operating_data_t *)arg;
	convert_u32_to_big_endian(&report_data->systick);
	convert_u32_to_big_endian(&report_data->cur_location);
	convert_u16_to_big_endian(&report_data->cur_speed);
	convert_u16_to_big_endian(&report_data->lidar_state);
	convert_u32_to_big_endian(&report_data->head_battery_vol);
	convert_u32_to_big_endian(&report_data->tail_battery_vol);
	data_len = mqtt_package_data(REPORT_REALTIME_DATA, (uint8_t *)report_data, sizeof(Report_realtime_operating_data_t), send_buf);
    return data_len;
}

static uint16_t send_report_package_infromation_msg(void *arg,uint8_t *send_buf)
{
    uint8_t i = 0;
    uint16_t len = sizeof(Report_package_information_t);
    uint16_t data_len = 0;
    Report_package_information_t package_sn = {0};
    Set_package_informtion_t *package_info = (Set_package_informtion_t *)arg;
    for(i = SLAVE_01;i<SLAVE_END;i++)
    {
        if(package_info[i].trailer_number)
        {
            package_sn.bits_sn |= 1<<(i-1);
            memcpy(package_data + len, package_info[i].SN.sn, SN_SIZE);
            len += SN_SIZE;
        }
    }
	convert_u16_to_big_endian(&package_sn.bits_sn);
	convert_u16_to_big_endian(&package_sn.padding);	
	memcpy(package_data, &package_sn, sizeof(Report_package_information_t));
	data_len = mqtt_package_data(REPORT_PACKAGE_INFORMATION, package_data, len, send_buf);
    return data_len;
}

static uint16_t send_report_alarm_data_msg(void *arg,uint8_t *send_buf)
{
    uint16_t data_len = 0;
    Report_alarm_data_t *alarm_data = (Report_alarm_data_t *)arg;
	convert_u32_to_big_endian(&alarm_data->car_run_state_alarm);
	convert_u32_to_big_endian(&alarm_data->drum_alarm);
	data_len = mqtt_package_data(REPORT_ALARM_DATA, (uint8_t *)alarm_data, sizeof(Report_alarm_data_t), send_buf);
    return data_len;
}

static int8_t mqtt_send_data_to_server(uint8_t *send_buf,uint16_t data_len)
{	
	return mqtt_client_send_data((char *)send_buf, data_len);
}


static void mqtt_tx_timeout_timer_cb(TimerHandle_t xTimer)
{
	/*tx线程会被tcp_write函数阻塞，到这需要关闭连接，然后重新建立连接*/
	if(mqtt_client_connected_or_not() == pdTRUE)
	{
		mqtt_disconnect_to_server();
		print_mqtt_tx_log("\r\nmqtt send timeout, disconnect to server, wait for reconnect!!\r\n");
	}
	print_mqtt_tx_log("\r\nmqtt_tx_timeout_timer_cb is called\r\n");
}

static void mqtt_connect_timeout_timer_cb(TimerHandle_t xTimer)
{
	print_mqtt_tx_log("\r\nmqtt connect timeout,wait next connect!!!\r\n");
	s_mqtt_connect_count = 0;
}


static int32_t creat_mqtt_timeout_timer(void)
{	
	mqtt_tx_timeout_timer_Handle = xTimerCreate((const char*)"mqtt_tx_timeout_timer",
									 (TickType_t			)(MQTT_RECONNECT_PERIOD + 1200),
									 (UBaseType_t			)pdFALSE,
									 (void*					)0,
									 (TimerCallbackFunction_t)mqtt_tx_timeout_timer_cb); //3s单次定时器
									 
	mqtt_connect_timeout_timer_Handle = xTimerCreate((const char*)"mqtt_connect_timeout_timer",
										(TickType_t				)(MQTT_CONNECT_TIMEOUT_TIME + (MQTT_RECONNECT_PERIOD * MQTT_CONNECT_TIMEOUT_COUNT)),
										(UBaseType_t			)pdFALSE,
										(void*					)1,
										(TimerCallbackFunction_t)mqtt_connect_timeout_timer_cb); //20单次定时器								 
    return 0;
}


void task_mqtt_tx(void const *arg)
{
	uint8_t first_send_get_para = 1, first_send_alarm_data = 1;
	uint32_t tick_cur, tick_report_sn = 0, tick_report_car_data = 0, tick_reconnect = 0, tick_print = 0;
	uint32_t tick_alive = 0;
	uint16_t data_len = 0;
    
	
    s_car_run_data_dev = find_data_manager("car_run_data");
	s_car_alarm_dev = find_data_manager("car_alarm_data");
    s_package_info_dev = find_data_manager("package_info");
    s_record_data_dev = find_data_manager("record_data");
    get_data_manager_data(s_record_data_dev, &s_record_data, sizeof(Record_operating_data_t));
    s_record_data.connect_times ++;
    set_single_data_flash_param("connect_times",&(s_record_data.connect_times),sizeof(s_record_data.connect_times));
    print_mqtt_tx_log("mqtt connect_times: %d\r\n",s_record_data.connect_times);
    
	osDelay(5000); //等待网络初始化成功
	creat_mqtt_timeout_timer();
	xTimerStop(mqtt_tx_timeout_timer_Handle, 0); 	
	mqtt_client_init(s_record_data.connect_times);
	osDelay(MQTT_RECONNECT_PERIOD);
	
	while(1)
	{
		tick_cur = get_systick_time();
		if(tick_cur - tick_alive >= (TASK_ALIVE_PRINT_PERIOD + 400))
		{
			tick_alive = tick_cur;
			print_mqtt_tx_log("\r\ntask mqtt_tx is aliving!!!\r\n");
		}
        get_data_manager_data(s_car_run_data_dev, &s_car_run_data, sizeof(Report_realtime_operating_data_t));
		get_data_manager_data(s_car_alarm_dev, &s_car_alarm_data, sizeof(Report_alarm_data_t));
        get_data_manager_data(s_package_info_dev, s_package_information, (sizeof(Set_package_informtion_t)<<4));
        
		if(mqtt_client_connected_or_not() == pdTRUE)
		{
						
//			for(uint8_t i = 0; i < sizeof(s_mqtt_send_handle_struct) / sizeof(*s_mqtt_send_handle_struct); i++)
//			{
//				send_data_flag = 0;
//				if(tick_cur - s_mqtt_send_handle_struct[i].tick_pre >= s_mqtt_send_handle_struct[i].interval)
//				{
//					s_mqtt_send_handle_struct[i].tick_pre = tick_cur;
//					if(s_mqtt_send_handle_struct[i].data_type == GET_OPERATING_PARA || s_mqtt_send_handle_struct[i].data_type == REPORT_ALARM_DATA)
//					{
////						if(first_send_get_para && s_mqtt_send_handle_struct[i].data_type == GET_OPERATING_PARA)
////						{
////							s_mqtt_send_handle_struct[i].send_data_handle();
////							first_send_get_para = 0;
////							send_data_flag = 1;
////						}
////						else if(s_mqtt_send_handle_struct[i].data_type == REPORT_ALARM_DATA)
////						{
////							if(memcmp(&s_car_alarm_data, &cmp_car_alarm_data, sizeof(Report_alarm_data_t)) || first_send_alarm_data)
////							{
//									memcpy(&cmp_car_alarm_data, &s_car_alarm_data, sizeof(Report_alarm_data_t));
//////								s_mqtt_send_handle_struct[i].send_data_handle();
//									first_send_alarm_data = 0;
////								send_data_flag = 1;
////							}
////						}
//					}
//					else
//					{
//						s_mqtt_send_handle_struct[i].send_data_handle();
//						send_data_flag = 1;
//					}
//					if(send_data_flag == 1)
//					{
//						mqtt_send_data_to_server();
//					}
//				}
//			}

			//单元测试
			/*第一次连接服务器发送获取参数报文*/
			if(first_send_get_para)
			{
				first_send_get_para = 0;
				data_len = s_mqtt_send_handle_struct[0].send_data_handle(&s_request_para,s_mqtt_send_data_buf);
				mqtt_send_data_to_server(s_mqtt_send_data_buf,data_len);
				xTimerStop(mqtt_connect_timeout_timer_Handle, 0);//停止连接超时定时器
			}
//			/*间隔100ms发送小车运行状态*/
			if(tick_cur - tick_report_car_data >= 300)
			{
				tick_report_car_data = tick_cur;
				data_len = s_mqtt_send_handle_struct[1].send_data_handle(&s_car_run_data,s_mqtt_send_data_buf);
				mqtt_send_data_to_server(s_mqtt_send_data_buf,data_len);
				xTimerStop(mqtt_tx_timeout_timer_Handle, 0); 	
				xTimerStart(mqtt_tx_timeout_timer_Handle, 0);
			}
			/*间隔1s上报包裹信息*/
			if(tick_cur - tick_report_sn >= 1000)
			{
				tick_report_sn = tick_cur;
				data_len = s_mqtt_send_handle_struct[2].send_data_handle(s_package_information,s_mqtt_send_data_buf);
				mqtt_send_data_to_server(s_mqtt_send_data_buf,data_len);
				
			}
			/*第一次连接服务器、出现报警或者报警消失立即上传报警信息*/
            if(memcmp(&s_car_alarm_data, &cmp_car_alarm_data, sizeof(Report_alarm_data_t)) || first_send_alarm_data)
            {
                memcpy(&cmp_car_alarm_data, &s_car_alarm_data, sizeof(Report_alarm_data_t));
                data_len = s_mqtt_send_handle_struct[3].send_data_handle(&s_car_alarm_data,s_mqtt_send_data_buf);
                first_send_alarm_data = 0;
                mqtt_send_data_to_server(s_mqtt_send_data_buf,data_len);
            }
			if(tick_cur - tick_print >= 5000)
			{
				tick_print = tick_cur;
				print_mqtt_tx_log("mqtt connect!!\r\n");
			}
		}
		else
		{
			if((s_mqtt_connect_count < MQTT_CONNECT_TIMEOUT_COUNT) && (tick_cur - tick_reconnect >= MQTT_RECONNECT_PERIOD))
			{
				tick_reconnect = tick_cur;
				s_mqtt_connect_count++;
				first_send_get_para = 1;
				first_send_alarm_data = 1;
				print_mqtt_tx_log("mqtt is disconnected!!connect_times: %d\r\n",s_record_data.connect_times);
				//尝试重连
                s_record_data.connect_times ++;
                set_single_data_flash_param("connect_times",&(s_record_data.connect_times),sizeof(s_record_data.connect_times));
				mqtt_client_init(s_record_data.connect_times);
				xTimerStop(mqtt_connect_timeout_timer_Handle, 0); 	
				xTimerStart(mqtt_connect_timeout_timer_Handle, 0);
			}
		}

		osDelay(50);
	}
}

