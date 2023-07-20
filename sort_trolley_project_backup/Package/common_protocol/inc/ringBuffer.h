#ifndef _GIZWITS_RING_BUFFER_H
#define _GIZWITS_RING_BUFFER_H

#ifdef __cplusplus
extern "C" {
#endif
	
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#define ICACHE_FLASH_ATTR 
#define min(a, b) (a)<(b)?(a):(b)                   ///< Calculate the minimum value

#pragma pack(1)
typedef struct {
    uint16_t rbCapacity;
    uint8_t  *rbHead;
    uint8_t  *rbTail;
    uint8_t  *rbBuff;
}rb_t;
#pragma pack()

int8_t rbCreate(rb_t* rb);
int8_t rbDelete(rb_t* rb);
int32_t rbCapacity(rb_t *rb);
int32_t rbCanRead(rb_t *rb);
int32_t rbCanWrite(rb_t *rb);
int32_t rbRead(rb_t *rb, void *data, size_t count);
int32_t rbWrite(rb_t *rb, const void *data, size_t count);

#ifdef __cplusplus
}
#endif
	
#endif
