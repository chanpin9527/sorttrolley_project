#include "lidar_driver.h"
#include "string.h"
#include "data_manager_data_type.h"

#pragma pack(1)

#define DEBUG_LIDAR_CONTROL  // printf
#ifdef DEBUG_LIDAR_CONTROL
#define print_lidar_log(fmt, ...) print_log(fmt, ##__VA_ARGS__)
#else
#define print_lidar_log(fmt, ...)
#endif

static uint8_t lidar_data_block_arr[DATA_BLOCK_END] = {
	DATA_BLOCK(0),
	DATA_BLOCK(1),
	DATA_BLOCK(2),
	DATA_BLOCK(3),
	DATA_BLOCK(4),
	DATA_BLOCK(5),
	DATA_BLOCK(6),
	DATA_BLOCK(7),
	DATA_BLOCK(8),
	DATA_BLOCK(9),
	DATA_BLOCK(10),
	DATA_BLOCK(11)
};

static uint8_t lidar_dist_block_arr[DIST_BLOCK_END] = {
	DIST_BLOCK(0),
	DIST_BLOCK(1),
	DIST_BLOCK(2),
	DIST_BLOCK(3),
	DIST_BLOCK(4),
	DIST_BLOCK(5),
	DIST_BLOCK(6),
	DIST_BLOCK(7),
	DIST_BLOCK(8),
	DIST_BLOCK(9),
	DIST_BLOCK(10),
	DIST_BLOCK(11),
	DIST_BLOCK(12),
	DIST_BLOCK(13),
	DIST_BLOCK(14),
	DIST_BLOCK(15)
};

static Lidar_data_t lidar_data_arr[LIDAR_VALID_DATA_SIZE] = {0};

static uint16_t get_data_block_azimuth(const uint8_t *lidar_data_in_buf, uint8_t data_block_id)
{
	uint32_t data_offset = 0;
	if(data_block_id < DATA_BLOCK_START || data_block_id > DATA_BLOCK_END)
		return 0;
	
	data_offset = DATA_FRAME_SIZE + lidar_data_block_arr[data_block_id] * DATA_BLOCK_SIZE;
	return (lidar_data_in_buf[data_offset] | (uint16_t)(lidar_data_in_buf[data_offset + 1]) << 8);
}

static void get_data_block_dist_and_rssi(const uint8_t *lidar_data_in_buf, uint8_t data_block_id, uint8_t dist_block_id, Lidar_data_t *lidar_data)
{
	uint32_t data_offset = 0;
	
	if(data_block_id < DATA_BLOCK_START || data_block_id > DATA_BLOCK_END || dist_block_id < DIST_BLOCK_START || dist_block_id > DIST_BLOCK_END)
		return ;
	
	data_offset = DATA_FRAME_SIZE + AZIMUTH_SIZE + lidar_data_block_arr[data_block_id] * DATA_BLOCK_SIZE + \
				lidar_dist_block_arr[dist_block_id] * (DOUBLE_DIST * (DIST_SIZE + RSSI_SIZE));
	lidar_data->dist = lidar_data_in_buf[data_offset] | (uint16_t)(lidar_data_in_buf[data_offset + 1]) << 8;
	lidar_data->rssi = lidar_data_in_buf[data_offset + DIST_SIZE];
}

#if 1
int8_t analysis_lidar_data(const uint8_t *lidar_data_in_buf, process_lidardist_func_t process_lidardist_handler)
{
	static uint32_t resolution = LIDAR_RESULOTION;
	uint16_t angle = 0;
	uint16_t azimuth0 = 0, azimuth1 = 0;
	uint16_t frame_head = 0;
	uint8_t i = 0, j = 0;
	Lidar_data_t lidar_data = {0};
	uint8_t lidar_valid_data_num = 0;
	
	if(lidar_data_in_buf == NULL)
		return -1;
	
	azimuth0 = get_data_block_azimuth(lidar_data_in_buf, DATA_BLOCK_0);
	azimuth1 = get_data_block_azimuth(lidar_data_in_buf, DATA_BLOCK_1);
	if(azimuth1 - azimuth0 > 0)
	{
		resolution = (azimuth1 - azimuth0) / DIST_NUM;
	}
	for(i = DATA_BLOCK_START; i < DATA_BLOCK_END; i++)
	{
		frame_head = lidar_data_in_buf[i*DATA_BLOCK_SIZE] | (uint16_t)(lidar_data_in_buf[i*DATA_BLOCK_SIZE+1]) << 8;
		if(frame_head == VALID_DATA_FRAME)
		{
			for(j = DIST_BLOCK_START; j < DIST_BLOCK_END; j++)
			{
				angle = get_data_block_azimuth(lidar_data_in_buf, i) + (resolution * j);
				if(angle != 0)
				{
					get_data_block_dist_and_rssi(lidar_data_in_buf, i, j, &lidar_data); //获取距离
					if(lidar_data.dist != 0)
					{
						lidar_data.angle = angle;//获得角度
						lidar_data_arr[lidar_valid_data_num++] = lidar_data;
					}
					else
					{
						//0.2度，中间会有一个0.1度间隔数据是0
					}
				}
				else
				{
//					goto data_send;
					/*无效数据*/
				}
			}
		}
		else
		{
			
//			if(i == 0) //一整个数据包都是无效数据
//				goto data_send;
			continue;
		}
	}
data_send:
	if(process_lidardist_handler != NULL)
	{
		process_lidardist_handler(lidar_data_arr, lidar_valid_data_num);
	}
	return 0;
}
#endif


#pragma pack()


