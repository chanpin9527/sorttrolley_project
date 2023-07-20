#include "mqtt_protocol.h"
#include "data_manager_data_type.h"
#include "string.h"
#include "convert_endian.h"


int32_t analysis_mqtt_protocol(uint8_t *data_in, uint16_t data_length, process_mqtt_protocol_func_t process_protocol_handler)
{
	int32_t ret = 0;
	uint16_t i = 0;
	Mqtt_data_head_t recv_data;
	uint16_t protocol_head_data_lenght = sizeof(Mqtt_data_head_t);
	
	if(data_in == NULL || data_length == 0)
		return -1;
	for(i = 0; i < data_length - protocol_head_data_lenght; i++)
	{
		if((data_in[i] == MQTT_VERSION) && (data_in[i + 1] == MQTT_SOURCE))
		{
			memcpy(&recv_data, (data_in + i), protocol_head_data_lenght);
			convert_u16_to_big_endian(&recv_data.head_data.data_type);
			convert_u16_to_big_endian(&recv_data.head_data.msg_id);
			convert_u16_to_big_endian(&recv_data.head_data.data_length);
			if(recv_data.head_data.data_length <= MQTT_PACKET_DATA_LENGTH_MAX && \
				((recv_data.head_data.data_length + i + protocol_head_data_lenght) <= data_length) && \
				(recv_data.head_data.data_type <= REPORT_ALARM_DATA) && \
				(process_protocol_handler != NULL))
			{
				process_protocol_handler(data_in + i + protocol_head_data_lenght, recv_data.head_data.data_length, recv_data.head_data.data_type);
				i += protocol_head_data_lenght + recv_data.head_data.data_length;
			}
			ret = i;
		}
		
	}	
	return ret;
}


int32_t  mqtt_package_data(uint16_t data_type, uint8_t *data_in, uint16_t data_len, uint8_t *data_out) 
{
	uint16_t index = 0;
	static uint16_t mag_id = 0;
    Mqtt_data_head_t mqtt_data_struct = {0};
	
    if(data_len > MQTT_PACKET_DATA_LENGTH_MAX)
    {
    	return -1;
    }
    
	mqtt_data_struct.head_data.version = MQTT_VERSION;
	mqtt_data_struct.head_data.source = MQTT_SOURCE;
	mqtt_data_struct.head_data.data_type = data_type;
	convert_u16_to_big_endian(&mqtt_data_struct.head_data.data_type);
	mqtt_data_struct.head_data.msg_id = mag_id ++;
	convert_u16_to_big_endian(&mqtt_data_struct.head_data.msg_id);
	mqtt_data_struct.head_data.data_length = data_len;
	convert_u16_to_big_endian(&mqtt_data_struct.head_data.data_length);
	
	memcpy(data_out, &mqtt_data_struct, sizeof(mqtt_data_struct));
	index += sizeof(mqtt_data_struct);
    
	if(data_len) 
    {
		memcpy(&data_out[index], data_in, data_len);
		index += data_len;
	}
    
    return index;
}

