#include "main.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "string.h"
#include "stdio.h"
#include "cmsis_os.h"
#include "eth_tcp_driver.h"
#include "common_protocol.h"

#pragma pack(1)

//#define DEBUG_TCP_CONTROL  // printf
#ifdef DEBUG_TCP_CONTROL
#define print_tcp_log(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define print_tcp_log(fmt, ...)
#endif

/***********************************************************************
函数名称：tcp_server_recv(void *arg, struct tcp_pcb *pcb,struct pbuf *p,err_t err)
功    能：TCP数据接收和发送
注    意：这是一个回调函数，当一个TCP段到达这个连接时会被调用
***********************************************************************/
#define OTA_TCP_SERVER_PORT		12345

#define TCP_RX_BUFFER_SIZE	32
rb_t g_tcp_rx_rb, *g_ptr_tcp_rx_rb = NULL;
uint8_t g_tcp_rx_buffer[TCP_RX_BUFFER_SIZE] = {0};
static uint8_t s_tcp_rx_buffer[TCP_RX_BUFFER_SIZE] = {0};

struct tcp_pcb *Compcb;
struct tcp_pcb *send_pcb;

static err_t tcp_server_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
	struct pbuf *p_temp = p;
	int ret;
	
	if(p_temp != NULL)
	{	
		tcp_recved(pcb, p_temp->tot_len);//获取数据长度 tot_len：tcp数据块的长度
		while(p_temp != NULL)	
		{
			Compcb = pcb;
 			tcp_write(pcb,p_temp->payload,p_temp->len,TCP_WRITE_FLAG_COPY); 	// payload为TCP数据块的起始位置       
 			tcp_output(pcb);
			ret = rbWrite(g_ptr_tcp_rx_rb, p_temp->payload, p_temp->len);
			memset(s_tcp_rx_buffer, 0, TCP_RX_BUFFER_SIZE);
			memcpy(s_tcp_rx_buffer, p_temp->payload, p_temp->len);
			
			print_tcp_log("\r\nTCP_rx_data:---%x-%x-%x---\r\n", 
			g_tcp_rx_buffer[0], g_tcp_rx_buffer[1],g_tcp_rx_buffer[2]);
			print_tcp_log("tcp receive %d bytes, rbWrite %d bytes\r\n", p_temp->len, ret);
		
			p_temp = p_temp->next;
		}
	}
	else
	{
		tcp_close(pcb); 											/* 作为TCP服务器不应主动关闭这个连接？ */
	}
	/* 释放该TCP段 */
	pbuf_free(p); 	
	err = ERR_OK;
	return err;
}


/***********************************************************************
函数名称：tcp_server_accept(void *arg, struct tcp_pcb *pcb,struct pbuf *p,err_t err)
功    能：回调函数
注    意：这是一个回调函数，当一个连接已经接受时会被调用
***********************************************************************/
static err_t tcp_server_accept(void *arg,struct tcp_pcb *pcb,err_t err)
{
	tcp_setprio(pcb, TCP_PRIO_MIN); 			/* 设置回调函数优先级，当存在几个连接时特别重要,此函数必须调用*/
	tcp_recv(pcb, tcp_server_recv); 			/* 设置TCP段到时的回调函数 */
	err = ERR_OK;
	
	send_pcb = pcb;	
	return err;
}


/***********************************************************************
函数名称：TCP_server_init(void)
功    能：完成TCP服务器的初始化，主要是使得TCP通讯快进入监听状态

***********************************************************************/
void OTA_tcp_server_init(void)
{
	struct tcp_pcb *pcb;

	memset(send_pcb, 0, sizeof(*send_pcb));
	
	g_ptr_tcp_rx_rb = init_rb_buf(&g_tcp_rx_rb, g_tcp_rx_buffer, TCP_RX_BUFFER_SIZE);//初始化缓存队列
	
	pcb = tcp_new(); 									/* 建立通信的TCP控制块(pcb) */
	tcp_bind(pcb, IP_ADDR_ANY, OTA_TCP_SERVER_PORT); 	/* 绑定本地IP地址和端口号（作为tcp服务器） */
	pcb = tcp_listen(pcb); 								/* 进入监听状态 */
	tcp_accept(pcb, tcp_server_accept); 			    /* 设置有连接请求时的回调函数 */
}

int8_t OTA_tcp_send(uint8_t *data, uint8_t len) 
{
	tcp_write(send_pcb, data, len, TCP_WRITE_FLAG_COPY);
	tcp_output(send_pcb);

	return 0;
}

#pragma pack()


