#ifndef __APP_MQTT_RX_H_
#define __APP_MQTT_RX_H_

#include "stdint.h"

int32_t init_mqtt_rx_msg_mq(void);

int mqtt_recv_data_process(void *arg, char *rec_buf, uint64_t buf_len);

void task_mqtt_rx(const void *arg);
	

#endif


