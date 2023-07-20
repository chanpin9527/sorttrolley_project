#ifndef __ETH_UDP_DRIVER_H_
#define __ETH_UDP_DRIVER_H_

#pragma pack(1)

#include "lwip/udp.h"
#include "stdint.h"
#include "data_manager_data_type.h"

typedef struct
{
	uint16_t frame_head;
	uint8_t lidar_data[UDP_DATA_SIZE];
}Lidar_inital_data_t;

void udp_send_data(struct udp_pcb *upcb, const char *send_data);

void udp_server_init(void);

#pragma pack()

#endif



