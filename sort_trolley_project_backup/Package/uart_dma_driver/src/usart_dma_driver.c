#include "usart_dma_driver.h"
#include "cmsis_os.h"
#include "string.h"
#include "stdio.h"
#include "main.h"


#pragma pack(1) //内存字节对齐

#define DEBUG_USART_DRIVER  // printf
#ifdef DEBUG_USART_DRIVER
#define print_uart_log(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define print_uart_log(fmt, ...)
#endif

typedef struct
{
	UART_HandleTypeDef  * _usartx;
	DMA_HandleTypeDef   * _usart_rx_dma_channel;
    uint8_t             * _buffer;
    uint32_t              _buffer_size;
    uint32_t              _head;
    uint32_t              _tail;
    uint8_t               _use_dma;
    usart_irq_callback_t  usart_cb; //中断回调函数
}Usart_dma_rx_config_struct_type;

typedef struct
{
	UART_HandleTypeDef  * _usartx;
	DMA_HandleTypeDef   * _usart_tx_dma_channel;
    uint8_t             * _buffer;
    uint32_t              _buffer_size;
    uint8_t               _send_finish;
}Usart_dma_tx_config_struct_type;

typedef struct
{
    uint8_t _index;
    osMutexId data_mutex;            // 数据保护互斥量
}Usart_struct_type;

#ifdef _USE_USART1_DMA
static uint8_t usart1_rx_buffer[USART1_TRANSFER_BUFF_SIZE] = {0};
static uint8_t usart1_tx_buffer[USART1_TRANSFER_BUFF_SIZE] = {0};
#else
//static uint8_t usart1_rx_buffer[1] = {0};
//static uint8_t usart1_tx_buffer[1] = {0};
#endif
#ifdef _USE_USART2_DMA
static uint8_t usart2_rx_buffer[USART2_TRANSFER_BUFF_SIZE] = {0};
static uint8_t usart2_tx_buffer[USART2_TRANSFER_BUFF_SIZE] = {0};
#else
static uint8_t usart2_rx_buffer[1] = {0};
static uint8_t usart2_tx_buffer[1] = {0};
#endif
#ifdef _USE_USART3_DMA
static uint8_t usart3_rx_buffer[USART3_TRANSFER_BUFF_SIZE] = {0};
static uint8_t usart3_tx_buffer[USART3_TRANSFER_BUFF_SIZE] = {0};
#else
//static uint8_t usart3_rx_buffer[1] = {0};
//static uint8_t usart3_tx_buffer[1] = {0};
#endif
#ifdef _USE_USART4_DMA
static uint8_t usart4_rx_buffer[USART4_TRANSFER_BUFF_SIZE] = {0};
static uint8_t usart4_tx_buffer[USART4_TRANSFER_BUFF_SIZE] = {0};
#else
//static uint8_t usart4_rx_buffer[1] = {0};
//static uint8_t usart4_tx_buffer[1] = {0};
#endif
#ifdef _USE_USART5_DMA
static uint8_t usart5_rx_buffer[USART5_TRANSFER_BUFF_SIZE] = {0};
static uint8_t usart5_tx_buffer[USART5_TRANSFER_BUFF_SIZE>>3] = {0};
#else
//static uint8_t usart5_rx_buffer[1] = {0};
//static uint8_t usart5_tx_buffer[1] = {0};
#endif
#ifdef _USE_USART6_DMA
static uint8_t usart6_rx_buffer[USART6_TRANSFER_BUFF_SIZE] = {0};
static uint8_t usart6_tx_buffer[USART6_TRANSFER_BUFF_SIZE] = {0};
#else
//static uint8_t usart6_rx_buffer[1] = {0};
//static uint8_t usart6_tx_buffer[1] = {0};
#endif
#ifdef _USE_USART7_DMA
static uint8_t usart7_rx_buffer[USART7_TRANSFER_BUFF_SIZE] = {0};
static uint8_t usart7_tx_buffer[USART7_TRANSFER_BUFF_SIZE] = {0};
#else
//static uint8_t usart7_rx_buffer[1] = {0};
//static uint8_t usart7_tx_buffer[1] = {0};
#endif
#ifdef _USE_USART8_DMA
static uint8_t usart8_rx_buffer[USART8_TRANSFER_BUFF_SIZE] = {0};
static uint8_t usart8_tx_buffer[USART8_TRANSFER_BUFF_SIZE] = {0};
#else
//static uint8_t usart8_rx_buffer[1] = {0};
//static uint8_t usart8_tx_buffer[1] = {0};
#endif

extern DMA_HandleTypeDef hdma_usart2_tx;

static Usart_dma_rx_config_struct_type usart_dma_rx_device[] = USART_DMA_RX_CONFIG_TABLE;
static Usart_dma_tx_config_struct_type usart_dma_tx_device[] = USART_DMA_TX_CONFIG_TABLE;
static Usart_struct_type usart_struct_type[] = USART_STRUCT_TABLE;

static void wait_send_finish(uint8_t usartx_n)
{
    uint32_t cur_tick = osKernelSysTick();
    uint32_t pre_tick = cur_tick;
    while(usart_dma_tx_device[usartx_n]._send_finish == 1)
    {
    	cur_tick = osKernelSysTick();
        if (cur_tick > pre_tick + 2)
        {
        	break;
        }
        osDelay(1);
    }
}


static void in_sending(uint8_t usartx_n)
{
    usart_dma_tx_device[usartx_n]._send_finish = 1;
}

static void in_no_sending(uint8_t usartx_n)
{
    usart_dma_tx_device[usartx_n]._send_finish = 0;
}

/**
  * @brief  Tx Transfer completed callback
  * @param  UartHandle: UART handle.
  * @note   This example shows a simple way to report end of DMA Tx transfer, and
  *         you can add your own implementation.
  * @retval None
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle)
{
  /* Set transmission flag: transfer complete */
	uint8_t i=0;
	for(i=_USART1_DEVICE_INDEX;i<_USART8_DEVICE_INDEX;i++)
	{
		if(UartHandle == usart_dma_tx_device[i]._usartx)
		{
			in_no_sending(i);
			break;
		}
	}
}

//void XferCpltCallback(DMA_HandleTypeDef * hdma)
//{
//	uint8_t i=0;
//	for(i=_USART1_DEVICE_INDEX;i<_USART8_DEVICE_INDEX;i++)
//	{
//		if(hdma == usart_dma_tx_device[i]._usart_tx_dma_channel)
//		{
//			in_no_sending(i);
//			break;
//		}
//	}
//}



uint32_t usartx_send_data_dma(uint8_t usartx_n , const void* buffer, uint16_t size)
{
    uint32_t temp_write_num = 0;
    if (size == 0)
        return 0;
    osMutexWait(usart_struct_type[usartx_n].data_mutex,osWaitForever);
    if (size > usart_dma_tx_device[usartx_n]._buffer_size)
    {
        temp_write_num = usart_dma_tx_device[usartx_n]._buffer_size;
    }
    else
    {
        temp_write_num = size;
    }

    wait_send_finish(usartx_n);
    in_sending(usartx_n);

    memcpy(usart_dma_tx_device[usartx_n]._buffer , buffer , temp_write_num);

    if(HAL_UART_Transmit_DMA(usart_dma_tx_device[usartx_n]._usartx,usart_dma_tx_device[usartx_n]._buffer,temp_write_num) != HAL_OK) 	//通过dma发送出去
    {
    	Error_Handler();
    	return 0;
    }
    osMutexRelease(usart_struct_type[usartx_n].data_mutex);
    return temp_write_num;
}

uint32_t usart_send_data(uint8_t usartx_n , const void* buffer, uint16_t size)
{
    uint32_t temp_write_num = 0;
    if (size == 0)
        return 0;
    if (size > usart_dma_tx_device[usartx_n]._buffer_size)
    {
        temp_write_num = usart_dma_tx_device[usartx_n]._buffer_size;
    }
    else
    {
        temp_write_num = size;
    }

    memcpy(usart_dma_tx_device[usartx_n]._buffer , buffer , temp_write_num);

	if(HAL_UART_Transmit(usart_dma_tx_device[usartx_n]._usartx,usart_dma_tx_device[usartx_n]._buffer,temp_write_num,10) != HAL_OK) 	//通过dma发送出去
    {
    	Error_Handler();
    	return 0;
    }
    return temp_write_num;
}

/***********************receive data***************************/
int32_t usartx_read_data(uint8_t usartx_n,void* buffer, uint16_t size)
{
	uint32_t get_num = 0;
	uint32_t temp_read_num = 0;
    osMutexWait(usart_struct_type[usartx_n].data_mutex,osWaitForever);
	/* 因为会有中断打断改变头尾的问题，所以增加临时变量 */
	uint32_t t_head = usart_dma_rx_device[usartx_n]._head;
	uint32_t t_tail = usart_dma_rx_device[usartx_n]._tail;
	if (t_tail != t_head)//有数据可读
	{
		if(t_tail < t_head)// 头部数据没有略过末尾
		{
			temp_read_num = t_head - t_tail;
			if (temp_read_num > size)
			{
				get_num = size;
			}
			else
			{
				get_num = temp_read_num;
			}
			memcpy(buffer ,&(usart_dma_rx_device[usartx_n]._buffer[t_tail]) , get_num);
			t_tail += get_num;
		}
		else
		{
			temp_read_num = usart_dma_rx_device[usartx_n]._buffer_size - t_tail;
			if (temp_read_num >= size)
			{
				get_num = size;
				memcpy(buffer ,&(usart_dma_rx_device[usartx_n]._buffer[t_tail]) , get_num);
				t_tail += get_num;
			}
			else
			{
				get_num = temp_read_num;
				memcpy(buffer ,&(usart_dma_rx_device[usartx_n]._buffer[t_tail]) , get_num);
				t_tail = 0;
				size -= get_num;
				if (t_head >= size)
				{
					memcpy((uint8_t *)buffer + get_num,&(usart_dma_rx_device[usartx_n]._buffer[t_tail]) , size);
					get_num += size;
					t_tail += size;
				}
				else
				{
					memcpy((uint8_t *)buffer + get_num,&(usart_dma_rx_device[usartx_n]._buffer[t_tail]) , t_head);
					get_num += t_head;
					t_tail += t_head;
				}
			}
		}
	}
	usart_dma_rx_device[usartx_n]._tail = t_tail;
    osMutexRelease(usart_struct_type[usartx_n].data_mutex);
	return get_num;
}

static void usartx_irq_get_data(uint8_t usartx_n)
{
    usart_dma_rx_device[usartx_n]._usartx->Instance->DR;
    usart_dma_rx_device[usartx_n]._head = usart_dma_rx_device[usartx_n]._buffer_size - __HAL_DMA_GET_COUNTER(usart_dma_rx_device[usartx_n]._usart_rx_dma_channel);
    if (usart_dma_rx_device[usartx_n]._head == usart_dma_rx_device[usartx_n]._buffer_size)
        usart_dma_rx_device[usartx_n]._head = 0;
}

static uint8_t Mycallback_handler(UART_HandleTypeDef *huart)
{
	uint8_t i=0;
	uint32_t length = 0;
	for(i=_USART1_DEVICE_INDEX;i<_USART8_DEVICE_INDEX;i++)
	{
		if((huart == usart_dma_rx_device[i]._usartx) && (usart_dma_rx_device[i]._use_dma == USE_DMA_YES))
		{
			usartx_irq_get_data(i);
			HAL_UART_Receive_DMA(usart_dma_rx_device[i]._usartx, usart_dma_rx_device[i]._buffer, usart_dma_rx_device[i]._buffer_size);
			if(usart_dma_rx_device[i]._head > usart_dma_rx_device[i]._tail)
			{
				length = usart_dma_rx_device[i]._head - usart_dma_rx_device[i]._tail;
			}
			else if(usart_dma_rx_device[i]._head < usart_dma_rx_device[i]._tail)
			{
				length = usart_dma_rx_device[i]._head + usart_dma_rx_device[i]._buffer_size - usart_dma_rx_device[i]._tail;
			}
			else
			{
				return 0;
			}
//			length = usartx_read_data(i,read_buffer,length);
			usart_dma_rx_device[i].usart_cb(length);
			break;
		}
	}
	return 0;
}

void Usart_Receive_Data_Idle(UART_HandleTypeDef *huart)
{
//	printf("isr:0x%08x\r\n",huart->Instance->ISR);
//	huart->Instance->RDR;
//	huart->Instance->RDR;
//	__HAL_UART_CLEAR_OREFLAG(huart);
	if(RESET != __HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE))   //判断是否是空闲中断
	{
		__HAL_UART_CLEAR_IDLEFLAG(huart);                     //清除空闲中断标志（否则会一直不断进入中断）
		Mycallback_handler(huart);                            //调用中断处理函数,这个函数自己写
	}
}

/***********************end***************************/
/**
 * @note      初始化串口 dma
 * @param     index 串口序号
 * @param     callback 回调函数
 * @return    返回状态已创建队列句柄
 */
int rt_hw_usart_dma_init(uint8_t usartx_n, usart_irq_callback_t callback)
{
	int32_t res = 0;
	if((usartx_n < _USART1_DEVICE_INDEX) || (usartx_n > _USART8_DEVICE_INDEX))
	{
		return -1;
	}
	osMutexDef(myMutex01);
	usart_struct_type[usartx_n].data_mutex = osMutexCreate(osMutex(myMutex01));
	usart_dma_rx_device[usartx_n].usart_cb = callback;
//	HAL_DMA_RegisterCallback(&hdma_usart2_tx, HAL_DMA_XFER_CPLT_CB_ID, XferCpltCallback);
	if(usart_dma_rx_device[usartx_n]._use_dma == USE_DMA_YES)
	{
		__HAL_UART_ENABLE_IT(usart_dma_rx_device[usartx_n]._usartx,UART_IT_IDLE);
		if(HAL_UART_Receive_DMA(usart_dma_rx_device[usartx_n]._usartx, usart_dma_rx_device[usartx_n]._buffer, usart_dma_rx_device[usartx_n]._buffer_size) != HAL_OK)
		{
			Error_Handler();
			res = -1;
		}
	}
	else
	{
		if(HAL_UART_Receive_IT(usart_dma_rx_device[usartx_n]._usartx,usart_dma_rx_device[usartx_n]._buffer, usart_dma_rx_device[usartx_n]._buffer_size) != HAL_OK)
		{
			Error_Handler();
			res = -1;
		}
	}
	return res;
}

/********************printf*************************/
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{         
	HAL_UART_Transmit(&huart3, (unsigned char *)&ch, 1, 1000);	
	return ch;
}

#pragma pack ()
