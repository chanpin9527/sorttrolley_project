#include "mqtt_driver.h"
#include "mqtt.h"
#include "string.h"
#include "stdio.h"
#include "cmsis_os.h"
#include "crypto_get_pass.h"
#include "system_tick.h"
#include "data_manager_data_type.h"
#include "led_driver.h"

#pragma pack(1)

#define DEBUG_MQTT_DRIVER  // printf
#ifdef DEBUG_MQTT_DRIVER
#define print_mqtt_driver_log(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define print_car_control_log(fmt, ...)
#endif

static mqtt_client_t* s_mqtt_client_handle; //�ͻ��˾��

#define MQTT_SERVER_IP_ARRD0		192
#define MQTT_SERVER_IP_ARRD1		168
#define MQTT_SERVER_IP_ARRD2		100
#define MQTT_SERVER_IP_ARRD3		100
#define MQTT_REMOTE_SERVER_PORT		1883

#define PRODUCT_NAME	"auto-distribute"		//��Ʒ����
#define SECURE_CODE		"65cMzS1nQXOYyUid"		//��ȫ��

#define DEVICE_NAME_LEN			15
#define UP_STR_LEN				35
#define	DOWN_STR_LEN			37

static Car_ip_t car_ip = {0};
static char device_name[DEVICE_NAME_LEN] = {0};
static char up_str[UP_STR_LEN] = {0};
static char down_str[DOWN_STR_LEN] = {0};

struct mqtt_connect_client_info_t mqtt_connect_info = {

                NULL,        				/* ������Ҫ�޸ģ�������ͬһ��������������ͬID�ᷢ����ͻ */ 
                NULL,                                /* MQTT �������û��� */
                NULL,                                /* MQTT ���������� */
                60,                                  /* �� MQTT ��������������ʱ�䣬ʱ�䳬��δ�������ݻ�Ͽ� */
                NULL,                         /* MQTT��������Ϣ����topic */
                NULL,                 /* MQTT��������Ϣ���Ͽ���������ʱ��ᷢ�� */
                0,                                   /* MQTT��������Ϣ Qos */
                0                                    /* MQTT��������Ϣ Retain */
    };

#define MQTT_BUFFER_SIZE		1024

typedef struct mqtt_recv
{
	char recv_buffer[MQTT_BUFFER_SIZE];
	uint16_t recv_len;  //��¼���ն��ٸ��ֽڵ�����
	uint16_t recv_total; //�����ݴ�С
}Mqtt_recv_data_t;

Mqtt_recv_data_t s_mqtt_recv_data = {.recv_len = 0, .recv_total = 0};

static err_t bsp_mqtt_subscribe(mqtt_client_t *mqtt_client, char *sub_topic, uint8_t qos);
static err_t mqtt_connect_server(uint32_t times);


static char *get_car_ip_str(Car_ip_t *car)
{
	get_car_ip(car);
	sprintf(device_name, "%04d-%02d%02d-%04d", car->area_id, car->rail_id1, car->rail_id2, car->car_id);
	return device_name;
}

static char *get_down(void)
{
	sprintf(down_str, "/%s/%s/%s", PRODUCT_NAME, device_name, "down");
	return down_str;
}

static char *get_up(void)
{
	sprintf(up_str, "/%s/%s/%s", PRODUCT_NAME, device_name, "up");
	return up_str;
}

/*!
* @brief MQTT ���ӳɹ��Ĵ���������Ҫ�Ļ���Ӧ�ò����¶���
*
* @param [in1] : MQTT ���Ӿ��
* @param [in2] : MQTT ���Ӳ���ָ��
*
* @retval: None
*/
void mqtt_connect_success_process(mqtt_client_t *client, void *arg)
{
    //���������ӳɹ�֮����� ���Ĳ���
    bsp_mqtt_subscribe(client, down_str, 0);               //���Ľӿں���
}

/*!
* @brief MQTT ����ʧ�ܵ��õĺ���
*
* @param [in1] : MQTT ���Ӿ��
* @param [in2] : MQTT ���Ӳ���ָ��
*
* @retval: None
*/
void mqtt_error_process_callback(mqtt_client_t *client, void *arg)
{
    /* ����������������������� clent ָ�����֪�����ĸ�MQTT�����Ӿ�� */
	print_mqtt_driver_log("\r\n------------------test reconnet to server------------------\r\n");
//	mqtt_client_free(client);
//	mqtt_connect_server();
}

/*!
* @brief mqtt �������ݴ������ӿڣ���Ҫ��Ӧ�ò���д���
* ִ��������mqtt���ӳɹ�
*
* @param [in1] : �û��ṩ�Ļص�����ָ��
* @param [in2] : ���յ�����ָ��
* @param [in3] : �������ݳ���
* @retval: ����Ľ��
*/
__weak int mqtt_recv_data_process(void *arg, char *rec_buf, uint64_t buf_len)
{
    print_mqtt_driver_log("recv_buffer = %s\n", rec_buf);
    return 0;
}


/*!
* @brief MQTT ���յ����ݵĻص�����
* ִ��������MQTT ���ӳɹ�
*
* @param [in1] : �û��ṩ�Ļص�����ָ��
* @param [in2] : MQTT �յ��ķְ�����ָ��
* @param [in3] : MQTT �ְ����ݳ���
* @param [in4] : MQTT ���ݰ��ı�־λ
* @retval: None
*/
static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags)
{
    if ((data == NULL) || (len == 0))
    {
        //���󷵻�
        print_mqtt_driver_log("mqtt_client_incoming_data_cb: condition error @entry\n");
        return;
    }

    if (s_mqtt_recv_data.recv_len + len < sizeof(s_mqtt_recv_data.recv_buffer))
    {
        //�����յ������ݼ���buffer��
		memcpy(&s_mqtt_recv_data.recv_buffer[s_mqtt_recv_data.recv_len], data, len);
//        snprintf(&s_mqtt_recv_data.recv_buffer[s_mqtt_recv_data.recv_len], len, "%s", data);
        s_mqtt_recv_data.recv_len += len;
    }

    if ((flags & MQTT_DATA_FLAG_LAST) == MQTT_DATA_FLAG_LAST) //���յ������ķְ����ݡ��������Ѿ����
    {
        mqtt_recv_data_process(arg, s_mqtt_recv_data.recv_buffer, s_mqtt_recv_data.recv_len);

        s_mqtt_recv_data.recv_len = 0;

        memset(s_mqtt_recv_data.recv_buffer, 0, sizeof(s_mqtt_recv_data.recv_buffer));
    }
    print_mqtt_driver_log("mqtt_client_incoming_data_cb:reveiving incomming data.\n");
}

/*!
* @brief MQTT ���յ����ݵĻص�����
* ִ��������MQTT ���ӳɹ�
*
* @param [in] : �û��ṩ�Ļص�����ָ��
* @param [in] : MQTT �յ����ݵ�topic
* @param [in] : MQTT �յ����ݵ��ܳ���
* @retval: None
*/
static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len)
{
    if ((topic == NULL) || (tot_len == 0))
    {
        print_mqtt_driver_log("bsp_mqtt_incoming_publish_cb: condition error @entry\n");
        return;
    }
    print_mqtt_driver_log("bsp_mqtt_incoming_publish_cb: topic = %s.\n", topic);
    print_mqtt_driver_log("bsp_mqtt_incoming_publish_cb: tot_len = %d.\n", tot_len);
    s_mqtt_recv_data.recv_total = tot_len; //��Ҫ���յ����ֽ�
    s_mqtt_recv_data.recv_len = 0;         //�ѽ����ֽڼ�����0

    //��ս���buffer
    memset(s_mqtt_recv_data.recv_buffer, 0, sizeof(s_mqtt_recv_data.recv_buffer));
}

/*!
* @brief MQTT ����״̬�Ļص�����
*
* @param [in] : MQTT ���Ӿ��
* @param [in] : �û��ṩ�Ļص�����ָ��
* @param [in] : MQTT ����״̬
* @retval: None
*/
static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status)
{
	print_mqtt_driver_log("mqtt_connection_cb: Enter\n");
    if (client == NULL)
    {
        print_mqtt_driver_log("mqtt_connection_cb: condition error @entry\n");
        return;
    }
    if (status == MQTT_CONNECT_ACCEPTED) //Successfully connected
    {
        print_mqtt_driver_log("bsp_mqtt_connection_cb: Successfully connected\n");

        // ע��������ݵĻص�����
        mqtt_set_inpub_callback(client, mqtt_incoming_publish_cb, mqtt_incoming_data_cb, arg);

        //���ӳɹ���������
        mqtt_connect_success_process(client, arg);
    }
    else //û�����ӳɹ���������������
    {
        print_mqtt_driver_log("bsp_mqtt_connection_cb: Fail connected, status = %s-%d\n", lwip_strerr(status), status);
        mqtt_error_process_callback(client, arg);
    }
}

/*!
* @brief ���ӵ� mqtt ������
* ִ����������
*
* @param [in] : None
*
* @retval: ����״̬��������ز��� ERR_OK ����Ҫ��������
*/
static err_t mqtt_connect_server(uint32_t times)
{
	ip_addr_t server_ip_addr;      /* Զ��ip��ַ */
	
    print_mqtt_driver_log("bsp_mqtt_connect: Enter!\n");
    err_t ret;
	
	IP4_ADDR(&server_ip_addr, MQTT_SERVER_IP_ARRD0, MQTT_SERVER_IP_ARRD1, MQTT_SERVER_IP_ARRD2, MQTT_SERVER_IP_ARRD3);
	
	
    if (s_mqtt_client_handle == NULL)
    {
        s_mqtt_client_handle = mqtt_client_new();
    }
    if (s_mqtt_client_handle == NULL)
    {
        print_mqtt_driver_log("bsp_mqtt_connect: s_mqtt_client_handle malloc fail @@!!!\n");
        return ERR_MEM;
    }
	
	get_car_ip_str(&car_ip);
	get_down();
	get_up();
	
//	print_mqtt_driver_log("----driver_name:%s--\r\n",device);
//    print_mqtt_driver_log("----up:%s--\r\n",up);
//    print_mqtt_driver_log("----down:%s--\r\n",down);
	
	mqtt_connect_info.client_id = generate_clientid(PRODUCT_NAME, device_name);
	mqtt_connect_info.client_user = generate_username(times, mqtt_connect_info.client_id);
	mqtt_connect_info.client_pass = generate_password(PRODUCT_NAME, device_name, SECURE_CODE);
    print_mqtt_driver_log("----client_id: %s\r\n",mqtt_connect_info.client_id);
    print_mqtt_driver_log("----client_user: %s\r\n",mqtt_connect_info.client_user);
    print_mqtt_driver_log("----client_pass: %s\r\n",mqtt_connect_info.client_pass);
	
    //��������,ע�⣺�����Ҫ���� arg ��arg������ȫ�ֱ������ֲ�����ָ��ᱻ���գ���ӣ���������
    ret = mqtt_client_connect(s_mqtt_client_handle, &server_ip_addr, MQTT_REMOTE_SERVER_PORT,\
                              mqtt_connection_cb, NULL, &mqtt_connect_info);
    /******************
	С��ʾ�����Ӵ�����Ҫ���κβ�����mqtt_client_connect ��ע��Ļص������������жϲ����ж�Ӧ�Ĳ���
	*****************/
//	print_mqtt_driver_log("bsp_mqtt_connect:ret_err:%d\r\n", ret);
    print_mqtt_driver_log("bsp_mqtt_connect: connect to mqtt %s-%d\n", lwip_strerr(ret), ret);
    return ret;
}





/*!
* @brief MQTT �������ݵĻص�����
* ִ��������MQTT ���ӳɹ�
*
* @param [in] : �û��ṩ�Ļص�����ָ��
* @param [in] : MQTT ���͵Ľ�����ɹ����߿��ܵĴ���
* @retval: None
*/
static void mqtt_client_pub_request_cb(void *arg, err_t result)
{
    //�������� arg ��ԭ
    mqtt_client_t *client = (mqtt_client_t *)arg;

    if (result != ERR_OK)
    {
        print_mqtt_driver_log("mqtt_client_pub_request_cb: c002: Publish FAIL, result = %s\n", lwip_strerr(result));

        //������
        mqtt_error_process_callback(client, arg);
    }
    else
    {
//        print_mqtt_driver_log("mqtt_client_pub_request_cb: c005: Publish complete!\n");
    }
}

/*!
* @brief ������Ϣ���������Ľӿں���
* ִ����������
*
* @param [in1] : mqtt ���Ӿ��
* @param [in2] : mqtt ���� topic ָ��
* @param [in3] : �������ݰ�ָ��
* @param [in4] : ���ݰ�����
* @param [in5] : qos
* @param [in6] : retain
* @retval: ����״̬
* @note: �п��ܷ��Ͳ��ɹ�������ʵ����ֵ�� 0 ����Ҫ�жϻص����� mqtt_client_pub_request_cb �Ƿ� result == ERR_OK
*/
static err_t bsp_mqtt_publish(mqtt_client_t *client, char *pub_topic, char *pub_buf, uint16_t data_len, uint8_t qos, uint8_t retain)
{
    if ((client == NULL) || (pub_topic == NULL) || (pub_buf == NULL) || \
    	(data_len == 0) || (qos > 2) || (retain > 1))
    {
        print_mqtt_driver_log("bsp_mqtt_publish: input error@@");
        return ERR_VAL;
    }

    //�ж��Ƿ�����״̬
    if (mqtt_client_is_connected(client) != pdTRUE)
    {
        print_mqtt_driver_log("bsp_mqtt_publish: client is not connected\n");
        return ERR_CONN;
    }

    err_t err;
#ifdef USE_MQTT_MUTEX
    // ���� mqtt ���ͻ�����
    if (s__mqtt_publish_mutex == NULL)
    {
        print_mqtt_driver_log("bsp_mqtt_publish: create mqtt mutex ! \n");
        s__mqtt_publish_mutex = xSemaphoreCreateMutex();
    }
    if (xSemaphoreTake(s__mqtt_publish_mutex, portMAX_DELAY) == pdPASS)
#endif /* USE_MQTT_MUTEX */
    {
        err = mqtt_publish(client, pub_topic, pub_buf, data_len,\
                           qos, retain, mqtt_client_pub_request_cb, (void *)client);
        if(err != ERR_OK)
        {
            print_mqtt_driver_log("bsp_mqtt_publish: mqtt_publish err = %s\n", lwip_strerr(err));
			/*����ʧ���Ƿ������Ͽ�����*/
//			mqtt_disconnect(client);
        }
#ifdef USE_MQTT_MUTEX
        print_mqtt_driver_log("bsp_mqtt_publish: mqtt_publish xSemaphoreTake\n");
        xSemaphoreGive(s__mqtt_publish_mutex);
#endif /* USE_MQTT_MUTEX */
    }
    return err;
}

err_t mqtt_client_send_data(char *send_data, uint16_t data_len)
{
	return bsp_mqtt_publish(s_mqtt_client_handle, up_str,\
                         send_data, data_len, 1, 0);
}


/*!
* @brief MQTT ���ĵĻص�����
* ִ��������MQTT ���ӳɹ�
*
* @param [in] : �û��ṩ�Ļص�����ָ��
* @param [in] : MQTT ���Ľ��
* @retval: None
*/
static void bsp_mqtt_request_cb(void *arg, err_t err)
{
    if (arg == NULL)
    {
        print_mqtt_driver_log("bsp_mqtt_request_cb: input error@@\n");
        return;
    }
    mqtt_client_t *client = (mqtt_client_t *)arg;
    if (err != ERR_OK)
    {
        print_mqtt_driver_log("bsp_mqtt_request_cb: FAIL sub, sub again, err = %s\n", lwip_strerr(err));
        //������
        mqtt_error_process_callback(client, arg);
    }
    else
    {
        print_mqtt_driver_log("bsp_mqtt_request_cb: sub SUCCESS!\n");
    }
}

/*!
* @brief mqtt ����
* ִ�����������ӳɹ�
*
* @param [in1] : mqtt ���Ӿ��
* @param [in2] : mqtt ���� topic ָ��
* @param [in5] : qos
* @retval: ����״̬
*/
static err_t bsp_mqtt_subscribe(mqtt_client_t *mqtt_client, char *sub_topic, uint8_t qos)
{
    print_mqtt_driver_log("bsp_mqtt_subscribe: Enter\n");

    if ((mqtt_client == NULL) || (sub_topic == NULL) || (qos > 2))
    {
        print_mqtt_driver_log("bsp_mqtt_subscribe: input error@@\n");
        return ERR_VAL;
    }
    if (mqtt_client_is_connected(mqtt_client) != pdTRUE)
    {
        print_mqtt_driver_log("bsp_mqtt_subscribe: mqtt is not connected, return ERR_CLSD.\n");
        return ERR_CLSD;
    }
    err_t err;

    //���ļ�ע��ص�����
    err = mqtt_subscribe(mqtt_client, sub_topic, qos, bsp_mqtt_request_cb, (void *)mqtt_client);
    if (err != ERR_OK)
    {
        print_mqtt_driver_log("bsp_mqtt_subscribe: mqtt_subscribe Fail, return:%s \n", lwip_strerr(err));
    }
    else
    {
        print_mqtt_driver_log("bsp_mqtt_subscribe: mqtt_subscribe SUCCESS, reason: %s\n", lwip_strerr(err));
    }
    return err;
}

uint8_t mqtt_client_connected_or_not(void)
{
	return mqtt_client_is_connected(s_mqtt_client_handle);
}


/*!
* @brief ��װ MQTT ��ʼ���ӿ�
* ִ����������
*
* @retval: ��
*/
void mqtt_client_init(uint32_t times)
{
    print_mqtt_driver_log("Mqtt init...\n");

    // ���ӷ�����
    mqtt_connect_server(times);
}

void mqtt_disconnect_to_server(void)
{
	mqtt_disconnect(s_mqtt_client_handle);
}


#pragma pack()

