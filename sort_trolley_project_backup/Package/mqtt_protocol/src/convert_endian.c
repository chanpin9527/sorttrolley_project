#include "convert_endian.h"

#pragma pack(1)
static void switch_u32_endian(uint32_t * data)
{
   *data = ((*data & 0xff000000) >> 24)
         | ((*data & 0x00ff0000) >>  8)
         | ((*data & 0x0000ff00) <<  8)
         | ((*data & 0x000000ff) << 24);
}

static void switch_u16_endian(uint16_t * data)
{
   *data = ((*data & 0x0000ff00) >>  8)
         | ((*data & 0x000000ff) <<  8);
}
static void switch_u64_endian(uint64_t * data)
{
   *data = ((*data & 0xff00000000000000) >> 56)
         | ((*data & 0x00ff000000000000) >> 40)
         | ((*data & 0x0000ff0000000000) >> 24)
		 | ((*data & 0x000000ff00000000) >>  8)
         | ((*data & 0x00000000ff000000) <<  8)
         | ((*data & 0x0000000000ff0000) << 24)
		 | ((*data & 0x000000000000ff00) << 40)
		 | ((*data & 0x00000000000000ff) << 56);
}

void convert_u32_to_little_endian(uint32_t * data)
{
	union{
		uint32_t data;
		uint8_t dataL[4];
	}test_endian_str;
	test_endian_str.data = 0x11223344;
	if(test_endian_str.dataL[0] == 0x11)//is big endian
		switch_u32_endian(data);
}

void convert_u32_to_big_endian(uint32_t * data)
{
	union{
		uint32_t data;
		uint8_t dataL[4];
	}test_endian_str;
	test_endian_str.data = 0x11223344;
	if(test_endian_str.dataL[0] == 0x44)//is little endian
		switch_u32_endian(data);
}

void convert_u16_to_little_endian(uint16_t * data)
{
	union{
		uint32_t data;
		uint8_t dataL[4];
	}test_endian_str;
	test_endian_str.data = 0x11223344;
	if(test_endian_str.dataL[0] == 0x11)//is big endian
		switch_u16_endian(data);
}

void convert_u16_to_big_endian(uint16_t * data)
{
	union{
		uint32_t data;
		uint8_t dataL[4];
	}test_endian_str;
	test_endian_str.data = 0x11223344;
	if(test_endian_str.dataL[0] == 0x44)//is little endian
		switch_u16_endian(data);
}
void convert_u64_to_little_endian(uint64_t * data)
{
	union{
		uint32_t data;
		uint8_t dataL[4];
	}test_endian_str;
	test_endian_str.data = 0x11223344;
	if(test_endian_str.dataL[0] == 0x11)//is big endian
		switch_u64_endian(data);
}

void convert_u64_to_big_endian(uint64_t * data)
{
	union{
		uint32_t data;
		uint8_t dataL[4];
	}test_endian_str;
	test_endian_str.data = 0x11223344;
	if(test_endian_str.dataL[0] == 0x44)//is little endian
		switch_u64_endian(data);
}
#pragma pack()
