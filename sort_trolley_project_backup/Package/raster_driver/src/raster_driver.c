#include "raster_driver.h"

#pragma pack(1)

#define DEBUG_RASTER_PROTOCOL  // printf
#ifdef DEBUG_RASTER_PROTOCOL
#define print_raster_log(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else	
#define print_raster_log(fmt, ...)
#endif


/**
 * @note      读计数光栅的状态
 * @param     None
 * @return    返回IO状态(高有效)
 */
uint8_t	read_counter_raster_statue(void)
{
	return COUNTER_RASTER_STATE;
}

/**
 * @note      读复位光栅的状态
 * @param     None
 * @return    返回IO状态(高有效)
 */
uint8_t	read_reset_raster_statue(void)
{
	return RESET_RASTER_STATE;
}


#pragma pack()
