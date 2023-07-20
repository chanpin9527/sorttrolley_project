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
�������ƣ�tcp_server_recv(void *arg, struct tcp_pcb *pcb,struct pbuf *p,err_t err)
��    �ܣ�TCP���ݽ��պͷ���
ע    �⣺����һ���ص���������һ��TCP�ε����������ʱ�ᱻ����
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
		tcp_recved(pcb, p_temp->tot_len);//��ȡ���ݳ��� tot_len��tcp���ݿ�ĳ���
		while(p_temp != NULL)	
		{
			Compcb = pcb;
 			tcp_write(pcb,p_temp->payload,p_temp->len,TCP_WRITE_FLAG_COPY); 	// payloadΪTCP���ݿ����ʼλ��       
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
		tcp_close(pcb); 											/* ��ΪTCP��������Ӧ�����ر�������ӣ� */
	}
	/* �ͷŸ�TCP�� */
	pbuf_free(p); 	
	err = ERR_OK;
	return err;
}


/***********************************************************************
�������ƣ�tcp_server_accept(void *arg, struct tcp_pcb *pcb,struct pbuf *p,err_t err)
��    �ܣ��ص�����
ע    �⣺����һ���ص���������һ�������Ѿ�����ʱ�ᱻ����
***********************************************************************/
static err_t tcp_server_accept(void *arg,struct tcp_pcb *pcb,err_t err)
{
	tcp_setprio(pcb, TCP_PRIO_MIN); 			/* ���ûص��������ȼ��������ڼ�������ʱ�ر���Ҫ,�˺����������*/
	tcp_recv(pcb, tcp_server_recv); 			/* ����TCP�ε�ʱ�Ļص����� */
	err = ERR_OK;
	
	send_pcb = pcb;	
	return err;
}


/***********************************************************************
�������ƣ�TCP_server_init(void)
��    �ܣ����TCP�������ĳ�ʼ������Ҫ��ʹ��TCPͨѶ��������״̬

***********************************************************************/
void OTA_tcp_server_init(void)
{
	struct tcp_pcb *pcb;

	memset(send_pcb, 0, sizeof(*send_pcb));
	
	g_ptr_tcp_rx_rb = init_rb_buf(&g_tcp_rx_rb, g_tcp_rx_buffer, TCP_RX_BUFFER_SIZE);//��ʼ���������
	
	pcb = tcp_new(); 									/* ����ͨ�ŵ�TCP���ƿ�(pcb) */
	tcp_bind(pcb, IP_ADDR_ANY, OTA_TCP_SERVER_PORT); 	/* �󶨱���IP��ַ�Ͷ˿ںţ���Ϊtcp�������� */
	pcb = tcp_listen(pcb); 								/* �������״̬ */
	tcp_accept(pcb, tcp_server_accept); 			    /* ��������������ʱ�Ļص����� */
}

int8_t OTA_tcp_send(uint8_t *data, uint8_t len) 
{
	tcp_write(send_pcb, data, len, TCP_WRITE_FLAG_COPY);
	tcp_output(send_pcb);

	return 0;
}

#pragma pack()


