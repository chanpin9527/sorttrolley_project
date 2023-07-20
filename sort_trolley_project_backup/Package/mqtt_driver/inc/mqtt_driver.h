#ifndef __MQTT_DRIVER_H_
#define __MQTT_DRIVER_H_

#include "stdint.h"
#include "mqtt.h"


void mqtt_client_init(uint32_t times);

err_t mqtt_client_send_data(char *send_data, uint16_t data_len);

void mqtt_disconnect_to_server(void);

uint8_t mqtt_client_connected_or_not(void);

#endif

