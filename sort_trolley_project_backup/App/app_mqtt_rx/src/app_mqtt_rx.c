#include "app_mqtt_rx.h"
#include "cmsis_os.h"
#include "string.h"
#include "stdio.h"
#include "data_manager.h"
#include "data_manager_data_type.h"
#include "mqtt_protocol.h"
#include "ringBuffer.h"
#include "common_protocol.h"
#include "convert_endian.h"
#include "flash_parameter.h"
#include "system_tick.h"

#define DEBUG_MQTT_RX  // printf
#ifdef DEBUG_MQTT_RX
#define print_mqtt_rx_log(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define print_mqtt_rx_log(fmt, ...)
#endif

//接收的数据
static Set_start_stop_cmd_t s_car_start_stop_cmd = {0};
static Set_package_informtion_t s_package_information[ROLLER_SIZE] = {0};
static Set_rail_operating_para_t s_rail_operating_para = {0};

static Data_manager_dev* s_package_info_dev = NULL;
static Data_manager_dev* s_operating_para_dev = NULL;

#define MQTT_RX_MSG_BUFF_SZIE		1024
static uint8_t s_mqtt_rx_msg_buf[MQTT_RX_MSG_BUFF_SZIE] = {0};
rb_t mqtt_rx_rb, *ptr_mqtt_rx_rb = NULL;


#define MQTT_MSG_BUFF_SIZE	256
typedef struct
{
	uint8_t msg[MQTT_MSG_BUFF_SIZE];
	uint16_t len;
}Mqtt_rx_mag_t;

#define MQTT_RX_MESSAGE_Q_NUM	10
static QueueHandle_t s_mqtt_rx_msg_mq;

#define START_STOP_CMD_MESSAGE_Q_NUM	3
QueueHandle_t s_start_stop_cmd_msg_mq;

osMutexId mqtt_rx_mutex;

int32_t init_mqtt_rx_msg_mq(void)
{
	s_mqtt_rx_msg_mq = xQueueCreate(MQTT_RX_MESSAGE_Q_NUM, sizeof(uint32_t));
    
	s_start_stop_cmd_msg_mq = xQueueCreate(START_STOP_CMD_MESSAGE_Q_NUM, sizeof(uint32_t));
	
	osMutexDef(myMutex);
	mqtt_rx_mutex = osMutexCreate(osMutex(myMutex));
	
	memset(s_mqtt_rx_msg_buf, 0x00, MQTT_RX_MSG_BUFF_SZIE);
	ptr_mqtt_rx_rb = init_rb_buf(&mqtt_rx_rb, s_mqtt_rx_msg_buf, MQTT_RX_MSG_BUFF_SZIE);
	
	return 0;
}

static void set_mqtt_rx_msg_mq(char *data, uint16_t len)
{
	BaseType_t err;
	uint32_t data_len = len;
	
	if((len <= 0) || (len > MQTT_MSG_BUFF_SIZE))
	{
		return;
	}
	
	osMutexWait(mqtt_rx_mutex, osWaitForever);
	rbWrite(ptr_mqtt_rx_rb, data, data_len);
    osMutexRelease(mqtt_rx_mutex);
	err = xQueueSendFromISR(s_mqtt_rx_msg_mq, &data_len, 0);
	if(err == errQUEUE_FULL)
	{
		print_mqtt_rx_log("s_mqtt_rx_msg_mq, os queue put failed!\r\n");
	}
    return;
}

int mqtt_recv_data_process(void *arg, char *rec_buf, uint64_t buf_len)
{
	set_mqtt_rx_msg_mq(rec_buf, buf_len);
	return buf_len;
}

static void process_mqtt_msg_handle(uint8_t *data, uint16_t data_len, uint16_t data_type)
{
	BaseType_t err;
    Set_package_informtion_t package_info = {0};
	if((data == NULL) || (data_len == 0))
		return ;
	
	switch(data_type)
	{
		case SET_START_STOP :
			if(data_len == sizeof(Set_start_stop_cmd_t))
			{
				memcpy(&s_car_start_stop_cmd, data, data_len);
				convert_u16_to_big_endian(&s_car_start_stop_cmd.start_stop_cmd);
				convert_u16_to_big_endian(&s_car_start_stop_cmd.padding);
                err = xQueueSend(s_start_stop_cmd_msg_mq, &s_car_start_stop_cmd, 0);
                print_mqtt_rx_log("*************receive start stop cmd:%d************\r\n",s_car_start_stop_cmd.start_stop_cmd);
                if(err == errQUEUE_FULL)
                {
                    print_mqtt_rx_log("s_mqtt_rx_msg_mq, os queue put failed!\r\n");
                }
			}
			break;
		case SET_PACKAGE_INFORMATION :
			if(data_len == sizeof(Set_package_informtion_t))
			{
                get_data_manager_data(s_package_info_dev, s_package_information, (sizeof(Set_package_informtion_t)<<4));
                memcpy(&package_info, data, data_len);
				convert_u32_to_big_endian(&package_info.gird_num);
                memcpy(&s_package_information[package_info.trailer_number], &package_info, data_len);
                set_package_info_flash_param((const char *)&package_info,package_info.trailer_number);
                print_mqtt_rx_log("package info number: %d,gird:%d,sn:%s\r\n",package_info.trailer_number,package_info.gird_num,package_info.SN.sn);
				if(set_data_manager_data(s_package_info_dev, s_package_information, (sizeof(Set_package_informtion_t)<<4))!= (sizeof(Set_package_informtion_t)<<4))
				{
					print_mqtt_rx_log("data manager set s_package_info_dev error\n");
				}
			}
			break;
		case SET_RAIL_OPERATING_PARA :
			if(data_len == sizeof(Set_rail_operating_para_t))
			{
				memcpy(&s_rail_operating_para, data, data_len);
				convert_u32_to_big_endian(&s_rail_operating_para.orbit_length);
				convert_u32_to_big_endian(&s_rail_operating_para.package_desk_length);
				convert_u16_to_big_endian(&s_rail_operating_para.normal_speed);
				convert_u16_to_big_endian(&s_rail_operating_para.enter_desk_speed);
				convert_u16_to_big_endian(&s_rail_operating_para.drum_parabola_speed);
				convert_u16_to_big_endian(&s_rail_operating_para.drum_parabola_distance);
                set_operating_para_flash_param((const char *)&s_rail_operating_para);
                print_mqtt_rx_log("orbit_length:%d,package_desk_length:%d,normal_speed:%d,desk_speed:%d,drum_speed:%d,drum_distance:%d\r\n",
                                                                               s_rail_operating_para.orbit_length,
                                                                               s_rail_operating_para.package_desk_length,
                                                                               s_rail_operating_para.normal_speed,
                                                                               s_rail_operating_para.enter_desk_speed,
                                                                               s_rail_operating_para.drum_parabola_speed,
                                                                               s_rail_operating_para.drum_parabola_distance);
				if(set_data_manager_data(s_operating_para_dev, &s_rail_operating_para, sizeof(Set_rail_operating_para_t))!= sizeof(Set_rail_operating_para_t))
				{
					print_mqtt_rx_log("data manager set s_rail_operating_para_dev error\n");
				}
			}
			break;
		default :
			break;
	}
}


void task_mqtt_rx(const void *arg)
{
	BaseType_t err;
	uint32_t tick_cur = 0, tick_alive = 0;
	Mqtt_rx_mag_t mqtt_rx_msg = {0};
    s_package_info_dev = find_data_manager("package_info");
    s_operating_para_dev = find_data_manager("operating_para");
	
	while(1)
	{
		tick_cur = get_systick_time();
		if(tick_cur - tick_alive >= (TASK_ALIVE_PRINT_PERIOD + 500))
		{
			tick_alive = tick_cur;
			print_mqtt_rx_log("\r\ntask mqtt_rx is aliving!!!\r\n");
		}
		err = xQueueReceive(s_mqtt_rx_msg_mq, &mqtt_rx_msg.len, 1000);
		if(pdTRUE == err)
		{
			osMutexWait(mqtt_rx_mutex, osWaitForever);
			rbRead(ptr_mqtt_rx_rb, mqtt_rx_msg.msg, mqtt_rx_msg.len);
			osMutexRelease(mqtt_rx_mutex);
			analysis_mqtt_protocol(mqtt_rx_msg.msg, mqtt_rx_msg.len, process_mqtt_msg_handle);
		}
	}
}

