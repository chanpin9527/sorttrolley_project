#include "common_protocol.h"
#include "ringBuffer.h"
#include "crc.h"
#include "convert_endian.h"

#pragma pack(1) //内存字节对齐

#define DEBUG_COMMON_PROTOCOL  // printf
#ifdef DEBUG_COMMON_PROTOCOL
//#define print_common_log(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define print_common_log(fmt, ...)
#endif

/**
// * @note      大小端切换
// * @param     *data 数据
// * @return    None
// */
//static void switch_u16_endian(uint16_t * data)
//{
//   *data = ((*data & 0x0000ff00) >>  8)
//         | ((*data & 0x000000ff) <<  8);
//}

/**
 * @note      判断大小端
 * @param     *data 数据
 * @return    None
 */
//void convert_u16_to_big_endian(uint16_t * data)
//{
//	union{
//		uint32_t data;
//		uint8_t dataL[4];
//	}test_endian_str;
//	test_endian_str.data = 0x11223344;
//	if(test_endian_str.dataL[0] == 0x44)//is little endian
//		switch_u16_endian(data);
//}

/**
 * @note      初始化消息队列
 * @param     *rb  待创建句柄
 * @param     *buf  缓冲buf
 * @param     len  长度
 * @return    已创建队列句柄
 */
rb_t *init_rb_buf(rb_t *rb,uint8_t *buf,uint32_t len)
{
	int ret = -1;
    
	rb->rbCapacity = len;
	rb->rbBuff = buf;
	ret = rbCreate(rb);
	if(0 != ret)
	{
//		print_common_log("In %s rbCreate fail\r\n", __func__);
	}else
	{
//		print_common_log("In %s rbCreate successful\r\n", __func__);
	}
	return rb;
}

/**
 * @note      协议解析函数
 * @param     *data_tmp 缓存
 * @param     length 长度
 * @param     process_protocol_handler 处理函数
 * @return    执行结果
 */
int32_t analysis_common_protocol(uint8_t *data_tmp,uint16_t length,process_protocol_func_t process_protocol_handler)
{
	uint8_t temp = 0,crc8_cal = 0,crc8_rev = 0;
	uint16_t i = 0, data_length = length;
    Link_data_t link_data_struct = {0};
    uint16_t link_data_struct_length = sizeof(Link_data_t);

	for (i = 0; i < data_length - link_data_struct_length; i++)
    {
		temp = data_tmp[i];
        if(temp == PACKET_HEAD)
        {
            memcpy(&link_data_struct,(data_tmp+i),link_data_struct_length);
            convert_u16_to_big_endian(&link_data_struct.mean_str.data_length);
            if((link_data_struct.mean_str.data_length <= UART_TRX_RB_MAX_LEN)
                && (i+link_data_struct.mean_str.data_length+link_data_struct_length <= data_length)
                && (link_data_struct.mean_str.data_offset == PACKET_DATA_OFFSET_BYTES)
                && (link_data_struct.mean_str.data_type <= PACKET_DATA_TYPE_MAX))
            {
                crc8_rev = data_tmp[i+link_data_struct.mean_str.data_length + link_data_struct_length];
                crc8_cal = crc8_16((data_tmp+i),link_data_struct.mean_str.data_length + link_data_struct_length);
                if(crc8_rev == crc8_cal)
                {
                	if(process_protocol_handler != NULL)
                	{
//                		print_common_log("process_protocol_handler\r\n");
                		process_protocol_handler(data_tmp+i+link_data_struct_length,link_data_struct.mean_str.data_length,link_data_struct.mean_str.data_type);
                		i += link_data_struct.mean_str.data_length + link_data_struct_length;
                	}
                }
            }
        }
	}
    return 1;
}

/**
 * @note      压包数据
 * @param     frame_type 类型
 * @param     *data_out 输出buf
 * @param     *data_in 输入buf
 * @param     data_len 输入数据长度
 * @return    None
 */
int32_t  package_data_to_protocol(uint8_t data_type, uint8_t *data_out, uint8_t *data_in,uint16_t data_len) 
{
	uint16_t index = 0,len = 0;
    Link_data_t link_data_struct = {0};
    if(data_len > PACKET_DATA_LENGTH_MAX)
    {
    	return -1;
    }
    
    link_data_struct.mean_str.ptorocol_head = PACKET_HEAD;
    link_data_struct.mean_str.data_length = data_len;
    link_data_struct.mean_str.data_type = data_type;
    link_data_struct.mean_str.data_offset = PACKET_DATA_OFFSET_BYTES;
    convert_u16_to_big_endian(&link_data_struct.mean_str.data_length);
    
    memcpy(data_out,&link_data_struct,sizeof(Link_data_t));
    index += sizeof(Link_data_t);
	
	if(data_len) 
    {
		memcpy(&data_out[index], data_in, data_len);
		index += data_len;
	}
    len = index;
	data_out[index++] = crc8_16(data_out, len);
    return index;
}
#pragma pack ()
