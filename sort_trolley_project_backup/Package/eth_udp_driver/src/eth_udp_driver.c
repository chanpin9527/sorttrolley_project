/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "string.h"
#include "stdio.h"
#include "cmsis_os.h"
#include "eth_udp_driver.h"
#include "common_protocol.h"
#include "data_manager_data_type.h"
#include "data_manager.h"


#pragma pack(1)

#define DEBUG_UDP_CONTROL  // printf
#ifdef DEBUG_UDP_CONTROL
#define print_udp_log(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else 
#define print_udp_log(fmt, ...)
#endif

extern QueueHandle_t lidar_init_data_msg_mq;
static Lidar_inital_data_t s_lidar_inital_data = {0};

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define UDP_SERVER_PORT 2368 /* define the UDP local connection port */
#define UDP_CLIENT_PORT 2368 /* define the UDP remote connection port */
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void udp_receive_data_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);
/* Private functions ---------------------------------------------------------*/
/**
* @brief Initialize the server application.
* @param None
* @retval None
*/
void udp_server_init(void)
{
	err_t err;
	struct udp_pcb *upcb;
	
	/* Create a new UDP control block */
	upcb = udp_new();
	if (upcb)
	{
		/* Bind the upcb to the UDP_PORT port */
		/* Using IP_ADDR_ANY allow the upcb to be used by any local interface */
		err = udp_bind(upcb, IP_ADDR_ANY, UDP_SERVER_PORT);
		if(err == ERR_OK)
		{
			/* Set a receive callback for the upcb */
			udp_recv(upcb, udp_receive_data_callback, NULL);
		}
		else
		{
			udp_remove(upcb);
		}
	}
}
/**
* @brief This function is called when an UDP datagrm has been received on the port UDP_PORT.
* @param arg user supplied argument (udp_pcb.recv_arg)
* @param pcb the udp_pcb which received data
* @param p the packet buffer that was received
* @param addr the remote IP address from which the packet was received
* @param port the remote port from which the packet was received
* @retval None
*/
static void udp_receive_data_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
	struct pbuf *p_temp = p;
	uint16_t frame = 0;
	
	/* Connect to the remote client */
	udp_connect(upcb, addr, UDP_CLIENT_PORT);
	
	if(p_temp != NULL)
	{	
		while(p_temp != NULL)	
		{
			frame = *((char *)(p_temp->payload) + 2) | (uint16_t)(*((char *)(p_temp->payload) + 3)) << 8;
			memcpy(&(s_lidar_inital_data.frame_head), &frame, sizeof(frame));
			memcpy(&(s_lidar_inital_data.lidar_data[0]), p_temp->payload, p_temp->len);
			xQueueSendFromISR(lidar_init_data_msg_mq, &s_lidar_inital_data, 0); //写入雷达初始数据队列
			p_temp = p_temp->next;
		}
	}
	
//	/* Tell the client that we have accepted it */
//	udp_send(upcb, p);
	/* free the UDP connection, so we can accept new clients */
	udp_disconnect(upcb);
	/* Free the p buffer */
	pbuf_free(p);
}
/* __MINIMAL_ECHO_H */

void udp_send_data(struct udp_pcb *upcb, const char *send_data)
{
	struct pbuf *ptr;
	uint32_t data_len = 0;
	
	data_len = strlen(send_data);
	ptr=pbuf_alloc(PBUF_TRANSPORT,data_len,PBUF_POOL); //申请内存
	if(ptr)
	{
		pbuf_take(ptr,(char*)send_data,data_len); //将数据打包进pbuf结构中
		udp_send(upcb,ptr);	//udp发送数据 
		pbuf_free(ptr);//释放内存
	} 
}

#pragma pack()

