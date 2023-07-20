#include "raster_driver.h"

#pragma pack(1)

#define DEBUG_RASTER_PROTOCOL  // printf
#ifdef DEBUG_RASTER_PROTOCOL
#define print_raster_log(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else	
#define print_raster_log(fmt, ...)
#endif


/**
 * @note      ��������դ��״̬
 * @param     None
 * @return    ����IO״̬(����Ч)
 */
uint8_t	read_counter_raster_statue(void)
{
	return COUNTER_RASTER_STATE;
}

/**
 * @note      ����λ��դ��״̬
 * @param     None
 * @return    ����IO״̬(����Ч)
 */
uint8_t	read_reset_raster_statue(void)
{
	return RESET_RASTER_STATE;
}


#pragma pack()
