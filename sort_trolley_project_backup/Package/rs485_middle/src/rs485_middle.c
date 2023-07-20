#include "rs485_middle.h"
#include "usart_dma_driver.h"
#include "usart.h"
#include "cmsis_os.h"

#pragma pack(1)

struct modbus_master *master;
//static uint8_t arg_buf[] = {0x95, 0x01, 0x64, 0x00, 0x6e, 0x4c, 0x32, 0x75};
//static uint8_t run_buf[] = {0x8a, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};

uint32_t rs485_send_data(const char *buffer, uint16_t length)
{	
	if(length == 0)
	{
		return 0;
	}
	
	RS485_TX_EN;
	length = usart_send_data(_USART2_DEVICE_INDEX, buffer, length);
	RS485_RX_EN;
	
	return length;
}

uint32_t rs485_read_data(uint8_t *buffer, uint16_t length)
{
	if(length == 0)
	{
		return 0;
	}
	return usartx_read_data(_USART2_DEVICE_INDEX, buffer, length);
}




//void task_rs485(void const *arg)
//{
//	osDelay(50000);
//	while(1)
//	{
//		rs485_send_data((const char *)arg_buf, sizeof(arg_buf) / sizeof(*arg_buf));
////		osDelay(2);
//		rs485_send_data((const char *)run_buf, sizeof(run_buf) / sizeof(*run_buf));
//		osDelay(15000);
//	}
//}

#pragma pack()
