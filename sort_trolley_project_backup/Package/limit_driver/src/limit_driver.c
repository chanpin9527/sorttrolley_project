#include "limit_driver.h"

#pragma pack(1)

#define DEBUG_LIMIT_PROTOCOL  // printf
#ifdef DEBUG_LIMIT_PROTOCOL
#define print_limit_log(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define print_limit_log(fmt, ...)
#endif

/**
 * @note      读限位开关的状态
 * @param     slave 1-15滚筒可选
 * @return    返回IO状态(高有效)
 */
uint8_t read_limit_state(uint8_t slave)
{
	uint8_t ret = 0;
	
	switch(slave)
	{
		case 1	: ret = LIMIT_STATE_READ(1);	break;
		case 2	: ret = LIMIT_STATE_READ(2);	break;
		case 3	: ret = LIMIT_STATE_READ(3);	break;
		case 4	: ret = LIMIT_STATE_READ(4);	break;
		case 5	: ret = LIMIT_STATE_READ(5);	break;
		case 6	: ret = LIMIT_STATE_READ(6);	break;
		case 7	: ret = LIMIT_STATE_READ(7);	break;
		case 8	: ret = LIMIT_STATE_READ(8);	break;
		case 9	: ret = LIMIT_STATE_READ(9);	break;
		case 10	: ret = LIMIT_STATE_READ(10);	break;
		case 11	: ret = LIMIT_STATE_READ(11);	break;
		case 12	: ret = LIMIT_STATE_READ(12);	break;
		case 13	: ret = LIMIT_STATE_READ(13);	break;
		case 14	: ret = LIMIT_STATE_READ(14);	break;
		case 15	: ret = LIMIT_STATE_READ(15);	break;
		default: break;
	}
	return ret;
}

#pragma pack()
