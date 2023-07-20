#include "rs485_driver.h"
#include "usart_dma_driver.h"
#include "data_manager.h"
#include "string.h"
#include "usart.h"


#pragma pack(1)

typedef void (*process_rx_data_func_t)(uint8_t, Fault_type);

/*******************************************************************************
*	运行类型：	单帧触发有ACK------0x84
*				单帧触发无ACK------0x94
*				双帧触发有ACK------0x85
*				双帧触发无ACK------0x95
*	从机ID:		0-31
*	运行方向：	0/1 正：FOREWORD（0）反：REVERSE（1）
*	运行速度：	0-0xff-----0-1530rpm
*	运行延时：	0-0xff-----value*0.01s
*	运行时间：	0-0xff-----value*0.01s
*	运行距离：	0-0xff-----value*0.01m
*	控制模式：	0/1	时间控制：TIME_CTL（1）位置控制：LOCTION_CTL（1）
*	PI值：		0-0x0f------1/16PI---1/16PI
*	加速度：	0-0x7f------1.5+value*0.05
*	说明：		单帧触发无ACK运行，可直接发送运行参数帧，直接驱动运行
*				单帧触发有ACK运行，可2ms后等待ACK
*				双帧触发无ACK运行，可发送运行参数帧后发送运行帧，驱动运行
*				双帧触发有ACK运行，可发送运行参数帧后2ms应答之后发送运行帧
*******************************************************************************/
typedef struct
{
	Run_type 		_run_type; 			//运行类型
	uint8_t 		_slaveID;			//从机id
	Run_dec			_run_direction; 	//运行方向
	uint8_t 		_run_speed;			//运行速度
	uint8_t 		_delay_run_time; 	//延迟运行时间
	uint8_t			_run_time;			//运行时间
	uint8_t			_run_distance;		//运行距离
	Control_mode	_ctl_mode;			//控制模式：时间控制or位置控制
	uint8_t			_vlauePI;			//pi值
	uint8_t			_acceleration;		//加速度
}Run_arg_struct_type;


#define RS485_TX_BUFF_SIZE	8
//static uint8_t s_arg_buffer[RS485_TX_BUFF_SIZE] = {0};
//static uint8_t s_run_buffer[RS485_TX_BUFF_SIZE] = {0};
static uint8_t s_run_type[] = {0x84, 0x94, 0x85, 0x95};


static Run_arg_struct_type	s_run_arg_struct_arr_left[SLAVE_END][MAXDEC] = {
		{
			{SINGLE_FRAME_NO_ACK, SLAVE_START, FOREWORD, 0, 0, 0, 0, LOCTION_CTL, 0, 0},
			{SINGLE_FRAME_NO_ACK, SLAVE_START, FOREWORD, 0, 0, 0, 0, LOCTION_CTL, 0, 0},
		},
		{
			{SINGLE_FRAME_NO_ACK, SLAVE_01, FOREWORD, RUN_SPEED, 0, 50, 115, LOCTION_CTL, 11, RUN_ACC},
			{SINGLE_FRAME_NO_ACK, SLAVE_01, FOREWORD, BACK_SPEED, 0, 50, 1, LOCTION_CTL, 11, BACK_ACC},
		},
		{
			{SINGLE_FRAME_NO_ACK, SLAVE_02, FOREWORD, RUN_SPEED, 0, 50, 115, LOCTION_CTL, 11, RUN_ACC},
			{SINGLE_FRAME_NO_ACK, SLAVE_02, FOREWORD, BACK_SPEED, 0, 50, 1, LOCTION_CTL, 11, BACK_ACC},
		},
		{
			{SINGLE_FRAME_NO_ACK, SLAVE_03, FOREWORD, RUN_SPEED, 0, 50, 115, LOCTION_CTL, 11, RUN_ACC},
			{SINGLE_FRAME_NO_ACK, SLAVE_03, FOREWORD, BACK_SPEED, 0, 50, 1, LOCTION_CTL, 11, BACK_ACC},
		},
		{
			{SINGLE_FRAME_NO_ACK, SLAVE_04, FOREWORD, RUN_SPEED, 0, 50, 115, LOCTION_CTL, 11, RUN_ACC},
			{SINGLE_FRAME_NO_ACK, SLAVE_04, FOREWORD, BACK_SPEED, 0, 50, 1, LOCTION_CTL, 11, BACK_ACC},
		},
		{
			{SINGLE_FRAME_NO_ACK, SLAVE_05, FOREWORD, RUN_SPEED, 0, 50, 115, LOCTION_CTL, 11, RUN_ACC},
			{SINGLE_FRAME_NO_ACK, SLAVE_05, FOREWORD, BACK_SPEED, 0, 50, 1, LOCTION_CTL, 11, BACK_ACC},
		},
		{
			{SINGLE_FRAME_NO_ACK, SLAVE_06, FOREWORD, RUN_SPEED, 0, 50, 115, LOCTION_CTL, 11, RUN_ACC},
			{SINGLE_FRAME_NO_ACK, SLAVE_06, FOREWORD, BACK_SPEED, 0, 50, 1, LOCTION_CTL, 11, BACK_ACC},
		},
		{
			{SINGLE_FRAME_NO_ACK, SLAVE_07, FOREWORD, RUN_SPEED, 0, 50, 115, LOCTION_CTL, 11, RUN_ACC},
			{SINGLE_FRAME_NO_ACK, SLAVE_07, FOREWORD, BACK_SPEED, 0, 50, 1, LOCTION_CTL, 11, BACK_ACC},
		},
		{
			{SINGLE_FRAME_NO_ACK, SLAVE_08, FOREWORD, RUN_SPEED, 0, 50, 115, LOCTION_CTL, 11, RUN_ACC},
			{SINGLE_FRAME_NO_ACK, SLAVE_08, FOREWORD, BACK_SPEED, 0, 50, 1, LOCTION_CTL, 11, BACK_ACC},
		},
		{
			{SINGLE_FRAME_NO_ACK, SLAVE_09, FOREWORD, RUN_SPEED, 0, 50, 115, LOCTION_CTL, 11, RUN_ACC},
			{SINGLE_FRAME_NO_ACK, SLAVE_09, FOREWORD, BACK_SPEED, 0, 50, 1, LOCTION_CTL, 11, BACK_ACC},
		},
		{
			{SINGLE_FRAME_NO_ACK, SLAVE_10, FOREWORD, RUN_SPEED, 0, 50, 115, LOCTION_CTL, 11, RUN_ACC},
			{SINGLE_FRAME_NO_ACK, SLAVE_10, FOREWORD, BACK_SPEED, 0, 50, 1, LOCTION_CTL, 11, BACK_ACC},
		},
		{
			{SINGLE_FRAME_NO_ACK, SLAVE_11, FOREWORD, RUN_SPEED, 0, 50, 120, LOCTION_CTL, 11, RUN_ACC},
			{SINGLE_FRAME_NO_ACK, SLAVE_11, FOREWORD, BACK_SPEED, 0, 50, 1, LOCTION_CTL, 11, BACK_ACC},
		},
		{
			{SINGLE_FRAME_NO_ACK, SLAVE_12, FOREWORD, RUN_SPEED, 0, 50, 115, LOCTION_CTL, 11, RUN_ACC},
			{SINGLE_FRAME_NO_ACK, SLAVE_12, FOREWORD, BACK_SPEED, 0, 50, 1, LOCTION_CTL, 11, BACK_ACC},
		},
		{
			{SINGLE_FRAME_NO_ACK, SLAVE_13, FOREWORD, RUN_SPEED, 0, 50, 115, LOCTION_CTL, 11, RUN_ACC},
			{SINGLE_FRAME_NO_ACK, SLAVE_13, FOREWORD, BACK_SPEED, 0, 50, 1, LOCTION_CTL, 11, BACK_ACC},
		},
		{
			{SINGLE_FRAME_NO_ACK, SLAVE_14, FOREWORD, RUN_SPEED, 0, 50, 115, LOCTION_CTL, 11, RUN_ACC},
			{SINGLE_FRAME_NO_ACK, SLAVE_14, FOREWORD, BACK_SPEED, 0, 50, 1, LOCTION_CTL, 11, BACK_ACC},
		},
		{
			{SINGLE_FRAME_NO_ACK, SLAVE_15, FOREWORD, RUN_SPEED, 0, 50, 115, LOCTION_CTL, 11, RUN_ACC},
			{SINGLE_FRAME_NO_ACK, SLAVE_15, FOREWORD, BACK_SPEED, 0, 50, 1, LOCTION_CTL, 11, BACK_ACC},
		},
};

static Run_arg_struct_type	s_run_arg_struct_arr_right[SLAVE_END][MAXDEC] = {
		{
			{SINGLE_FRAME_NO_ACK, SLAVE_START, BACKWORD, 0, 0, 0, 0, LOCTION_CTL, 0, 0},
			{SINGLE_FRAME_NO_ACK, SLAVE_START, BACKWORD, 0, 0, 0, 0, LOCTION_CTL, 0, 0},
		},
		{
			{SINGLE_FRAME_NO_ACK, SLAVE_01, BACKWORD, RUN_SPEED, 0, 50, 115, LOCTION_CTL, 11, RUN_ACC},
			{SINGLE_FRAME_NO_ACK, SLAVE_01, BACKWORD, BACK_SPEED, 0, 50, 1, LOCTION_CTL, 11, BACK_ACC},
		},
		{
			{SINGLE_FRAME_NO_ACK, SLAVE_02, BACKWORD, RUN_SPEED, 0, 50, 115, LOCTION_CTL, 11, RUN_ACC},
			{SINGLE_FRAME_NO_ACK, SLAVE_02, BACKWORD, BACK_SPEED, 0, 50, 1, LOCTION_CTL, 11, BACK_ACC},
		},
		{
			{SINGLE_FRAME_NO_ACK, SLAVE_03, BACKWORD, RUN_SPEED, 0, 50, 115, LOCTION_CTL, 11, RUN_ACC},
			{SINGLE_FRAME_NO_ACK, SLAVE_03, BACKWORD, BACK_SPEED, 0, 50, 1, LOCTION_CTL, 11, BACK_ACC},
		},
		{
			{SINGLE_FRAME_NO_ACK, SLAVE_04, BACKWORD, RUN_SPEED, 0, 50, 115, LOCTION_CTL, 11, RUN_ACC},
			{SINGLE_FRAME_NO_ACK, SLAVE_04, BACKWORD, BACK_SPEED, 0, 50, 1, LOCTION_CTL, 11, BACK_ACC},
		},
		{
			{SINGLE_FRAME_NO_ACK, SLAVE_05, BACKWORD, RUN_SPEED, 0, 50, 115, LOCTION_CTL, 11, RUN_ACC},
			{SINGLE_FRAME_NO_ACK, SLAVE_05, BACKWORD, BACK_SPEED, 0, 50, 1, LOCTION_CTL, 11, BACK_ACC},
		},
		{
			{SINGLE_FRAME_NO_ACK, SLAVE_06, BACKWORD, RUN_SPEED, 0, 50, 115, LOCTION_CTL, 11, RUN_ACC},
			{SINGLE_FRAME_NO_ACK, SLAVE_06, BACKWORD, BACK_SPEED, 0, 50, 1, LOCTION_CTL, 11, BACK_ACC},
		},
		{
			{SINGLE_FRAME_NO_ACK, SLAVE_07, BACKWORD, RUN_SPEED, 0, 50, 115, LOCTION_CTL, 11, RUN_ACC},
			{SINGLE_FRAME_NO_ACK, SLAVE_07, BACKWORD, BACK_SPEED, 0, 50, 1, LOCTION_CTL, 11, BACK_ACC},
		},
		{
			{SINGLE_FRAME_NO_ACK, SLAVE_08, BACKWORD, RUN_SPEED, 0, 50, 115, LOCTION_CTL, 11, RUN_ACC},
			{SINGLE_FRAME_NO_ACK, SLAVE_08, BACKWORD, BACK_SPEED, 0, 50, 1, LOCTION_CTL, 11, BACK_ACC},
		},
		{
			{SINGLE_FRAME_NO_ACK, SLAVE_09, BACKWORD, RUN_SPEED, 0, 50, 115, LOCTION_CTL, 11, RUN_ACC},
			{SINGLE_FRAME_NO_ACK, SLAVE_09, BACKWORD, BACK_SPEED, 0, 50, 1, LOCTION_CTL, 11, BACK_ACC},
		},
		{
			{SINGLE_FRAME_NO_ACK, SLAVE_10, BACKWORD, RUN_SPEED, 0, 50, 115, LOCTION_CTL, 11, RUN_ACC},
			{SINGLE_FRAME_NO_ACK, SLAVE_10, BACKWORD, BACK_SPEED, 0, 50, 1, LOCTION_CTL, 11, BACK_ACC},
		},
		{
			{SINGLE_FRAME_NO_ACK, SLAVE_11, BACKWORD, RUN_SPEED, 0, 50, 115, LOCTION_CTL, 11, RUN_ACC},
			{SINGLE_FRAME_NO_ACK, SLAVE_11, BACKWORD, BACK_SPEED, 0, 50, 1, LOCTION_CTL, 11, BACK_ACC},
		},
		{
			{SINGLE_FRAME_NO_ACK, SLAVE_12, BACKWORD, RUN_SPEED, 0, 50, 115, LOCTION_CTL, 11, RUN_ACC},
			{SINGLE_FRAME_NO_ACK, SLAVE_12, BACKWORD, BACK_SPEED, 0, 50, 1, LOCTION_CTL, 11, BACK_ACC},
		},
		{
			{SINGLE_FRAME_NO_ACK, SLAVE_13, BACKWORD, RUN_SPEED, 0, 50, 115, LOCTION_CTL, 11, RUN_ACC},
			{SINGLE_FRAME_NO_ACK, SLAVE_13, BACKWORD, BACK_SPEED, 0, 50, 1, LOCTION_CTL, 11, BACK_ACC},
		},
		{
			{SINGLE_FRAME_NO_ACK, SLAVE_14, BACKWORD, RUN_SPEED, 0, 50, 115, LOCTION_CTL, 11, RUN_ACC},
			{SINGLE_FRAME_NO_ACK, SLAVE_14, BACKWORD, BACK_SPEED, 0, 50, 1, LOCTION_CTL, 11, BACK_ACC},
		},
		{
			{SINGLE_FRAME_NO_ACK, SLAVE_15, BACKWORD, RUN_SPEED, 0, 50, 115, LOCTION_CTL, 11, RUN_ACC},
			{SINGLE_FRAME_NO_ACK, SLAVE_15, BACKWORD, BACK_SPEED, 0, 50, 1, LOCTION_CTL, 11, BACK_ACC},
		},
};


//static uint8_t register_data_crc(uint8_t *data_in, uint8_t *data_out)
//{
//	uint8_t i = 0, data_size = 0;
//	uint8_t crc_data = 0;
//	uint8_t write_flag = 0;
//	
//	write_flag = (data_in[3] >> 6) & 0x01;
//	if(write_flag == 0) //读数据
//	{
//		for(i = 1; i < 6; i++)
//		{
//			crc_data ^= data_in[i];
//		}
//		data_out[6] = crc_data;
//		memcpy(data_out, data_in, 6);
//	}
//	else
//	{
//		data_size = data_in[5];
//		for(i = 1; i < data_size * 2 + 6; i++)
//		{
//			crc_data ^= data_in[i];
//		}
//		data_out[data_size * 2 + 6] = crc_data;
//		memcpy(data_out, data_in, data_size * 2 + 6);
//	}
//	return crc_data;
//}
/**
 * @note      设置电机速度
 * @param     *data_in需要计算的数组
 * @param     *data_out需要输出的数组
 * @return    crc结果
 */
static uint8_t crc_data(uint8_t *data_in, uint8_t *data_out)
{
	uint8_t crc = 0, i = 0;
	
	for(i = 1; i <= 6; i++)
	{
		crc ^= data_in[i];
	}
	data_out[i] = crc;
	memcpy(data_out, data_in, i);
	
	return crc;
}

/**
 * @note      设置电机运行参数帧
 * @param     *run_arg参数
 * @param     *arg_bu输出的参数数组
 * @return    None
 */
static void set_arg_frame(Run_arg_struct_type *run_arg, uint8_t *arg_buf)
{
	uint8_t temp_buf[8] = {0};
	
	if(run_arg == NULL || arg_buf == NULL)
		return ;
	
	memset(temp_buf, 0x00, RS485_TX_BUFF_SIZE);
	
	temp_buf[0] = s_run_type[run_arg->_run_type]; //单帧or双帧
	
	temp_buf[1] = (run_arg->_run_direction << 6)  			\
				| ((!!(run_arg->_run_speed & 0x80)) << 5)  	\
				| (run_arg->_slaveID & 0x1f);
	
	temp_buf[2] = run_arg->_run_speed & 0x7f;
	
	temp_buf[3] = run_arg->_delay_run_time & 0x7f;
	
	if(run_arg->_ctl_mode == TIME_CTL)
	{
		temp_buf[4] = run_arg->_run_time & 0x7f;
	}
	else if(run_arg->_ctl_mode == LOCTION_CTL)
	{
		temp_buf[4] = run_arg->_run_distance & 0x7f;
	}
	
	temp_buf[5] = ((run_arg->_vlauePI & 0x0f) << 3)		 \
				| ((run_arg->_ctl_mode) << 2)			 \
				| ((!!(run_arg->_run_time & 0x80)) << 1) \
				| ((!!(run_arg->_delay_run_time & 0x80)));
	
	temp_buf[6] = run_arg->_acceleration & 0x7f;
	
	arg_buf[7] = crc_data(temp_buf, arg_buf);
}

/**
 * @note      设置电机运行帧
 * @param     slaveID从机ID
 * @param     *run_buf输出的运行数组
 * @return    None
 */
static void set_run_frame(uint8_t slaveID, uint8_t *run_buf)
{
	uint8_t temp_buf[RS485_TX_BUFF_SIZE] = {0};
	
	temp_buf[0] = RUN_FRAME_HEAD;
	
	if(slaveID < 1 || slaveID > 31)
	{
		return ;
	}
	else if(slaveID >= 1 && slaveID <= 7)
	{
		temp_buf[1] = 0x01 << (slaveID - 1);
	}
	else if(slaveID >= 9 && slaveID <= 15)
	{
		temp_buf[2] = 0x01 << (slaveID - 9);
	}
	else if(slaveID >= 17 && slaveID <= 23)
	{
		temp_buf[3] = 0x01 << (slaveID - 17);
	}
	else if(slaveID >= 25 && slaveID <= 31)
	{
		temp_buf[4] = 0x01 << (slaveID - 25);
	}
	else
	{
		if(slaveID == 8)
		{
			temp_buf[5] = 0x01;
		}
		else if(slaveID == 16)
		{
			temp_buf[5] = 0x01 << 1;
		}
		else
		{
			temp_buf[5] = 0x01 << 2;
		}
	}
	
	temp_buf[6] = RESERVE;
	
	run_buf[7] = crc_data(temp_buf, run_buf);
}

/**
 * @note      计算应答帧的crc
 * @param     *ack_buf应答帧数据
 * @return    crc值
 */
//static uint8_t crc_ack_data(uint8_t *ack_buf)
//{
//	if(ack_buf == NULL)
//	{
//		return  0;
//	}
//	
//	return ((ack_buf[1] ^ ack_buf[2]) & 0x7f);
//}

/**
 * @note      解析接受的485数据
 * @param     *rx_buf接受的数据
 * @param     length 数据长度
 * @param     pocess_rx_data_handler 处理数据的函数
 * @return    返回结果
 */
//static int8_t analysis_rs485_rx_data(uint8_t *rx_buf,uint8_t length, process_rx_data_func_t pocess_rx_data_handler)
//{
//	uint8_t slaveID = 0;
//	Fault_type fault;
//	
//	if(rx_buf == NULL || rx_buf[1] != ACK_FRAME_HEAD || rx_buf[3] != crc_ack_data(rx_buf))
//	{
//		return -1;
//	}
//	
//	slaveID = rx_buf[1] & 0x1f;
//	fault.fault_value = rx_buf[2] | ((!!(rx_buf[1] & 0x40)) << 7);
//	
//	if(pocess_rx_data_handler != NULL)
//	{
//		pocess_rx_data_handler(slaveID, fault);
//	}
//	
//	return 0;
//}

/**
 * @note      根据接收的485数据处理
 * @param     slaveID 从机ID
 * @param     fault 错误信息
 * @return    None
 */
//static void pocess_rx_data(uint8_t slaveID, Fault_type fault)
//{
//	/*
//	处理故障，上报故障信息
//	*/
//}

/**
 * @note      485发送数据
 * @param     *buffer要发送的数据
 * @param     length 数据长度
 * @return    发送的数据字节数
 */
static uint32_t rs485_send_data(const char *buffer, uint16_t length)
{	
	uint16_t len = 0, time_out = 0;
	
	if(length == 0)
	{
		return 0;
	}
	
	RS485_TX_EN;
	usartx_send_data_dma(_USART2_DEVICE_INDEX, buffer, length); //DMA发送数据
	/*轮询方式*/
//	if(HAL_UART_Transmit(&huart2, (const uint8_t *)buffer, length, 10) != HAL_OK)
//    {
//    	Error_Handler();
//    	return 0;
//    }
//	RS485_RX_EN;
	
	return len;
}

/**
 * @note      滚筒运行方式
 * @param     slaveID从机ID
 * @param     direction方向 FOREWORD前进 BACKWORD后退
 * @param     turn_dec旋转方向 LEFT_DEC左转 RIGHT_DEC右转
 * @return    None
 */
void drum_run_forword_backword(uint8_t slaveID, uint8_t direction, uint8_t turn_dec)
{
	uint8_t arg_buffer[RS485_TX_BUFF_SIZE] = {0}, run_buffer[RS485_TX_BUFF_SIZE] = {0};
	
	if(slaveID < SLAVE_START || slaveID > SLAVE_END || \
		(direction != FOREWORD && direction != BACKWORD) || \
		(turn_dec != LEFT_DEC && turn_dec != RIGHT_DEC))
		return ;
	if(turn_dec == LEFT_DEC)
	{
		set_arg_frame(&s_run_arg_struct_arr_left[slaveID][direction], arg_buffer); //设置运行参数帧
//		set_run_frame(slaveID, run_buffer); //设置运行帧
	}
	else
	{
		set_arg_frame(&s_run_arg_struct_arr_right[slaveID][direction], arg_buffer); //设置运行参数帧
//		set_run_frame(slaveID, run_buffer); //设置运行帧
	}
	
	rs485_send_data((const char *)arg_buffer, RS485_TX_BUFF_SIZE);
//	rs485_send_data((const char *)run_buffer, RS485_TX_BUFF_SIZE);
}



#pragma pack()
