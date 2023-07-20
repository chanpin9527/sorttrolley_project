#ifndef __ETH_TCP_DRIVER_H_
#define __ETH_TCP_DRIVER_H_

#include "stdint.h"

void OTA_tcp_server_init(void);

int8_t OTA_tcp_send(uint8_t *data, uint8_t len);

#endif

