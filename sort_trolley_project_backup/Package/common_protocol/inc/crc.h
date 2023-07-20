#ifndef __CRC_H__
#define __CRC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

uint16_t crc16( uint8_t * pucFrame, uint16_t usLen );
uint8_t crc8(uint8_t *data, uint8_t length);
uint8_t crc8_16(uint8_t *data, uint16_t length);
unsigned char crc8Verify(unsigned char *data, int length);
unsigned char crc8VerifyFF(unsigned char *data, int length);
#ifdef __cplusplus
}
#endif

#endif
