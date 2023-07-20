#ifndef __USART_DMA_DRIVER_H__
#define __USART_DMA_DRIVER_H__
#include "stm32f4xx_hal.h"
#include "usart.h"

typedef void (*usart_irq_callback_t)(uint32_t ); //中断回调函数



#define USE_DMA_YES   (1)
#define USE_DMA_NO    (0)

//#define _USE_USART1_DMA
#define _USE_USART2_DMA
//#define _USE_USART3_DMA
//#define _USE_USART4_DMA
//#define _USE_USART5_DMA
//#define _USE_USART6_DMA
//#define _USE_USART7_DMA
//#define _USE_USART8_DMA

#define USART2_TRANSFER_BUFF_SIZE     (1<<8)
#define USART3_TRANSFER_BUFF_SIZE     (1<<10)
#define USART5_TRANSFER_BUFF_SIZE     (1<<11)
#define USART6_TRANSFER_BUFF_SIZE     (1<<10)

enum
{
    _USART1_DEVICE_INDEX = 0,
    _USART2_DEVICE_INDEX = 1,
    _USART3_DEVICE_INDEX = 2,
    _USART4_DEVICE_INDEX = 3,
    _USART5_DEVICE_INDEX = 4,
    _USART6_DEVICE_INDEX = 5,
    _USART7_DEVICE_INDEX = 6,
    _USART8_DEVICE_INDEX = 7,
};

//extern DMA_HandleTypeDef hdma_usart2_rx;
//extern DMA_HandleTypeDef hdma_usart2_tx;

#define USART_DMA_RX_CONFIG_TABLE \
{\
    [_USART2_DEVICE_INDEX] = {&huart2, \
    						  NULL,\
                              usart2_rx_buffer,\
							  USART2_TRANSFER_BUFF_SIZE,\
                              0,\
                              0,\
							  USE_DMA_YES,\
                              NULL},\
}

#define USART_DMA_TX_CONFIG_TABLE \
{\
    [_USART2_DEVICE_INDEX] = {&huart2, \
    	                 	  &hdma_usart2_tx,\
                              usart2_tx_buffer,\
							  USART2_TRANSFER_BUFF_SIZE,\
                              0},\
}

#define USART_STRUCT_TABLE \
{\
    [_USART2_DEVICE_INDEX] = {._index = _USART2_DEVICE_INDEX},\
}

/**
 * @note      初始化串口3 dma
 * @param     None
 * @return    返回状态已创建队列句柄
 */
int rt_hw_usart_dma_init(uint8_t usart_n,usart_irq_callback_t callback);
uint32_t usartx_send_data_dma(uint8_t usartx_n , const void* buffer, uint16_t size);
uint32_t usart_send_data(uint8_t usartx_n , const void* buffer, uint16_t size);
int32_t usartx_read_data(uint8_t usartx_n,void* buffer, uint16_t size);

#endif	//__COMMON_PROTOCOL_H__
