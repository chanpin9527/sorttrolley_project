#ifndef __LIDAR_DRIVER_H_
#define __LIDAR_DRIVER_H_

#include "stdint.h"

#pragma pack(1)

typedef struct
{
	uint16_t dist;
	uint16_t angle;
	uint8_t rssi;
}Lidar_data_t;


typedef void(*process_lidardist_func_t)(Lidar_data_t *, uint8_t );

#define LIDAR_VALID_DATA_SIZE	384
#define DIST_NUM			16

#define DATA_BLOCK_SIZE		100
#define VALID_DATA_FRAME	((uint16_t)0xeeff)
#define DATA_FRAME_SIZE		2
#define AZIMUTH_SIZE		2
#define DOUBLE_DIST			2 //两组距离（最强回波和最后回波）
#define DIST_SIZE			2
#define RSSI_SIZE			1



#define DATA_BLOCK_0	0
#define DATA_BLOCK_1	1
#define DATA_BLOCK_2	2
#define DATA_BLOCK_3	3
#define DATA_BLOCK_4	4
#define DATA_BLOCK_5	5
#define DATA_BLOCK_6	6
#define DATA_BLOCK_7	7
#define DATA_BLOCK_8	8
#define DATA_BLOCK_9	9
#define DATA_BLOCK_10	10
#define DATA_BLOCK_11	11
#define DATA_BLOCK(n)	DATA_BLOCK_##n

#define DIST_BLOCK_0	0
#define DIST_BLOCK_1	1
#define DIST_BLOCK_2	2
#define DIST_BLOCK_3	3
#define DIST_BLOCK_4	4
#define DIST_BLOCK_5	5
#define DIST_BLOCK_6	6
#define DIST_BLOCK_7	7
#define DIST_BLOCK_8	8
#define DIST_BLOCK_9	9
#define DIST_BLOCK_10	10
#define DIST_BLOCK_11	11
#define DIST_BLOCK_12	12
#define DIST_BLOCK_13	13
#define DIST_BLOCK_14	14
#define DIST_BLOCK_15	15
#define DIST_BLOCK(n)	DIST_BLOCK_##n


enum DATA_BLOCK_NUM
{
	DATA_BLOCK_START = 0,
	DATA_BLOCK_END   = 12,
};

enum DIST_BLOCK_NUM
{
	DIST_BLOCK_START = 0,
	DIST_BLOCK_END   = 16,
};

int8_t analysis_lidar_data(const uint8_t *lidar_data_in_buf, process_lidardist_func_t process_lidardist_handler);

#pragma pack()

#endif

