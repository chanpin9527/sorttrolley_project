#ifndef _COPNVERT_ENDIAN_H_
#define _COPNVERT_ENDIAN_H_

#include <stdint.h>

void convert_u32_to_little_endian(uint32_t * data);
void convert_u32_to_big_endian(uint32_t * data);


void convert_u16_to_little_endian(uint16_t * data);
void convert_u16_to_big_endian(uint16_t * data);

void convert_u64_to_little_endian(uint64_t * data);
void convert_u64_to_big_endian(uint64_t * data);

#endif
