#ifndef __MQTT_PROTOCOL_H_
#define __MQTT_PROTOCOL_H_

#include "stdint.h"


typedef void(*process_mqtt_protocol_func_t)(uint8_t *, uint16_t, uint16_t);




/*
	协议格式：
	|  version  |    source   | 数据类型 |   数据ID |   数据长度 |  数据域  |
	|  (2)1byte |   (1)1byte  |  2byte   |   2byte  |   2byte    |	 变长	|
	*/
	
#define MQTT_PACKET_DATA_LENGTH_MAX	512
#define MQTT_VERSION				0x02
#define MQTT_SOURCE					0x01
#define MQTT_PROTOCOL_HEAD_LENGTH	8

typedef union
{
	struct
	{
		uint8_t version;
		uint8_t source;
		uint16_t data_type;
		uint16_t msg_id;
		uint16_t data_length;
	}head_data;
	uint8_t data[MQTT_PROTOCOL_HEAD_LENGTH];
}Mqtt_data_head_t;




int32_t analysis_mqtt_protocol(uint8_t *data_in, uint16_t data_length, process_mqtt_protocol_func_t process_protocol_handler);

int32_t  mqtt_package_data(uint16_t data_type, uint8_t *data_in, uint16_t data_len, uint8_t *data_out);

#endif

