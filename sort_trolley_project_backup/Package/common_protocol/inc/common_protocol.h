#ifndef __COMMON_PROTOCOL_H__
#define __COMMON_PROTOCOL_H__
#include <stdint.h>
#include "ringBuffer.h"

#pragma pack(1) //内存字节对齐
#define LINK_DATA_LENTH    (5)

typedef union
{
    struct{
        uint8_t ptorocol_head;
        uint16_t data_length;
        uint8_t data_offset;
        uint8_t data_type;
    }mean_str;
    uint8_t data[LINK_DATA_LENTH];
}Link_data_t;

/*
	协议格式：
	|  标识   | 数据长度 |  数据偏移 | 数据类型 |   数据域   |   校验和   |
	|  0x21   |   2byte  |   1byte   |  1byte   |   256byte  |   1byte    |
	*/
#define PROTOCOL_HEAD_OFF 			0
#define DATA_LEN_OFF      			1
#define DATA_LEN_OFF_HIGH			2
#define DATA_OFFSET_OFF   			3
#define DATA_TYPE_OFF     			4
#define VALID_DATA_OFF    			5

#define PACKET_HEAD 	        	0x21	/*protocol packet head*/
#define PACKET_DATA_OFFSET_BYTES    VALID_DATA_OFF	/*protocol data offset bytes*/

#define PACKET_DATA_LENGTH_MAX         (0x400)
#define PACKET_DATA_TYPE_MAX           (0x64)

#define UART_TRX_RB_MAX_LEN            (512)

#define CAN_STD_FRAME	1
#define CAN_EXT_FRAME	2
#define STD_ID_LEN		2
#define EXT_ID_LEN		4

typedef enum
{	/*protocol data type*/
	TYPE_CAN_OF_VCI = 0,             //0x00CAN转发          VCI→VMCU
	TYPE_POWER_CMD_TO_VCI,           //0x01电源管理指令        VMCU→VCI
	TYPE_POWER_RES_FROM_VCI,         //0x02电源管理反馈        VCI→VMCU
	TYPE_POWER_CMD_FROM_PMCU,        //0x03电源管理指令        PMCU→VMCU
	TYPE_POWER_RES_TO_PMCU,          //0x04电源管理反馈        VMCU→PMCU
	TYPE_SYN_CMD_FROM_PMCU,          //0x05同步指令           PMCU→VMCU
	TYPE_SYN_DATA_TO_PMCU,           //0x06同步状态给PMCU      VMCU→PMCU
	TYPE_SYN_CMD_TO_PMCU,            //0x07同步指令           VMCU→PMCU
	TYPE_SYN_DATA_FROM_PMCU,         //0x08PMCU同步信息       PMCU→VMCU
	TYPE_SYN_CMD_FROM_VCI,           //0x09同步指令           VCI→VMCU
	TYPE_SYN_DATA_TO_VCI,            //0x0A同步状态给VCI       VMCU→VCI
	TYPE_DIAGNOSIS_CMD,              //0x0B诊断指令           VCI→VMCU
	TYPE_DIAGNOSIS_RES,              //0x0C诊断反馈           VMCU→VCI
	TYPE_LOG_OF_VMCU,                //0x0DVMCU日志          VMCU→VCI
	TYPE_LOG_OF_5G_A,                //0x0E5G_A日志转发       VMCU→VCI
	TYPE_LOG_OF_5G_B,                //0x0F5G_B日志转发       VMCU→VCI
	TYPE_LED_DISPLAY_DATA,           //0x10LED显示屏数据下发    VCI→VMCU
	TYPE_LED_DISPLAY_RES,            //0x11LED屏反馈         VMCU→VCI
	TYPE_EMERGENCY_STOP_CMD,         //0x12急停指令           VCI→VMCU
	TYPE_EMERGENCY_STOP_RES,         //0x13急停反馈           VMCU→VCI
	TYPE_OTA_INFORM_CMD,             //0x14OTA 通知指令       VCI→VMCU
	TYPE_OTA_INFORM_RES,             //0x15升级通知反馈信息      VMCU→VCI
	TYPE_PROGRAM_CMD,                //0x16程序写入指令         VCI→VMCU
	TYPE_PROGRAM_RES,                //0x17程序写入反馈消息       VMCU→VCI
	TYPE_VERIFY_DATA_CMD,            //0x18校验指令            VCI→VMCU
	TYPE_VERIFY_DATA_RES,            //0x19校验反馈消息          VMCU→VCI
	TYPE_REBOOT_DEVICE,              //0x1A重启设备             VCI→VMCU
	TYPE_LED_CAN_BAUD_CMD,           //0x1Bled屏转发波特率设置&查询 VCI→VMCU
	TYPE_LED_CAN_BAUD_RES,           //0x1Cled屏波特率设置反馈     VMCU→VCI
	TYPE_LIGHT_SENSOR_DATA = 0x1D,   //0x1D光传感器电压值         VMCU→VCI
	TYPE_FRAME_MAX                   //
}Packet_data_type_t;


typedef int32_t (*process_protocol_func_t)(uint8_t *,uint16_t,uint8_t);

/**
 * @note      判断大小端
 * @param     *data 数据
 * @return    None
 */
//void convert_u16_to_big_endian(uint16_t * data);

/**
 * @note      初始化消息队列
 * @param     *rb  待创建句柄
 * @param     *buf  缓冲buf
 * @param     len  长度
 * @return    已创建队列句柄
 */
rb_t *init_rb_buf(rb_t *rb,uint8_t *buf,uint32_t len);

/**
 * @note      协议解析函数
 * @param     *rb 队列句柄
 * @param     process_protocol_handler 处理函数
 * @return    执行结果
 */
int32_t analysis_common_protocol(uint8_t *data_tmp,uint16_t length,process_protocol_func_t process_protocol_handler);

/**
 * @note      压包数据
 * @param     frame_type 类型
 * @param     *data_out 输出buf
 * @param     *data_in 输入buf
 * @param     data_len 输入数据长度
 * @return    None
 */
int32_t  package_data_to_protocol(uint8_t data_type, uint8_t *data_out, uint8_t *data_in,uint16_t data_len);

#pragma pack() //内存字节对齐
#endif	//__COMMON_PROTOCOL_H__
