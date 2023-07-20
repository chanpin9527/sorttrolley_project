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

static mqtt_client_t* s_mqtt_client_handle; //客户端句柄

#define MQTT_SERVER_IP_ARRD0		192
#define MQTT_SERVER_IP_ARRD1		168
#define MQTT_SERVER_IP_ARRD2		100
#define MQTT_SERVER_IP_ARRD3		100
#define MQTT_REMOTE_SERVER_PORT		1883

#define PRODUCT_NAME	"auto-distribute"		//产品名称
#define SECURE_CODE		"65cMzS1nQXOYyUid"		//安全码

#define DEVICE_NAME_LEN			15
#define UP_STR_LEN				35
#define	DOWN_STR_LEN			37

static Car_ip_t car_ip = {0};
static char device_name[DEVICE_NAME_LEN] = {0};
static char up_str[UP_STR_LEN] = {0};
static char down_str[DOWN_STR_LEN] = {0};

struct mqtt_connect_client_info_t mqtt_connect_info = {

                NULL,        				/* 这里需要修改，以免在同一个服务器两个相同ID会发生冲突 */ 
                NULL,                                /* MQTT 服务器用户名 */
                NULL,                                /* MQTT 服务器密码 */
                60,                                  /* 与 MQTT 服务器保持连接时间，时间超过未发送数据会断开 */
                NULL,                         /* MQTT遗嘱的消息发送topic */
                NULL,                 /* MQTT遗嘱的消息，断开服务器的时候会发送 */
                0,                                   /* MQTT遗嘱的消息 Qos */
                0                                    /* MQTT遗嘱的消息 Retain */
    };

#define MQTT_BUFFER_SIZE		1024

typedef struct mqtt_recv
{
	char recv_buffer[MQTT_BUFFER_SIZE];
	uint16_t recv_len;  //记录接收多少个字节的数据
	uint16_t recv_total; //总数据大小
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
* @brief MQTT 连接成功的处理函数，需要的话在应用层重新定义
*
* @param [in1] : MQTT 连接句柄
* @param [in2] : MQTT 连接参数指针
*
* @retval: None
*/
void mqtt_connect_success_process(mqtt_client_t *client, void *arg)
{
    //这里是连接成功之后进行 订阅操作
    bsp_mqtt_subscribe(client, down_str, 0);               //订阅接口函数
}

/*!
* @brief MQTT 处理失败调用的函数
*
* @param [in1] : MQTT 连接句柄
* @param [in2] : MQTT 连接参数指针
*
* @retval: None
*/
void mqtt_error_process_callback(mqtt_client_t *client, void *arg)
{
    /* 这里可以做重连操作，根据 clent 指针可以知道是哪个MQTT的连接句柄 */
	print_mqtt_driver_log("\r\n------------------test reconnet to server------------------\r\n");
//	mqtt_client_free(client);
//	mqtt_connect_server();
}

/*!
* @brief mqtt 接收数据处理函数接口，需要在应用层进行处理
* 执行条件：mqtt连接成功
*
* @param [in1] : 用户提供的回调参数指针
* @param [in2] : 接收的数据指针
* @param [in3] : 接收数据长度
* @retval: 处理的结果
*/
__weak int mqtt_recv_data_process(void *arg, char *rec_buf, uint64_t buf_len)
{
    print_mqtt_driver_log("recv_buffer = %s\n", rec_buf);
    return 0;
}


/*!
* @brief MQTT 接收到数据的回调函数
* 执行条件：MQTT 连接成功
*
* @param [in1] : 用户提供的回调参数指针
* @param [in2] : MQTT 收到的分包数据指针
* @param [in3] : MQTT 分包数据长度
* @param [in4] : MQTT 数据包的标志位
* @retval: None
*/
static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags)
{
    if ((data == NULL) || (len == 0))
    {
        //错误返回
        print_mqtt_driver_log("mqtt_client_incoming_data_cb: condition error @entry\n");
        return;
    }

    if (s_mqtt_recv_data.recv_len + len < sizeof(s_mqtt_recv_data.recv_buffer))
    {
        //将接收到的数据加入buffer中
		memcpy(&s_mqtt_recv_data.recv_buffer[s_mqtt_recv_data.recv_len], data, len);
//        snprintf(&s_mqtt_recv_data.recv_buffer[s_mqtt_recv_data.recv_len], len, "%s", data);
        s_mqtt_recv_data.recv_len += len;
    }

    if ((flags & MQTT_DATA_FLAG_LAST) == MQTT_DATA_FLAG_LAST) //接收的是最后的分包数据——接收已经完成
    {
        mqtt_recv_data_process(arg, s_mqtt_recv_data.recv_buffer, s_mqtt_recv_data.recv_len);

        s_mqtt_recv_data.recv_len = 0;

        memset(s_mqtt_recv_data.recv_buffer, 0, sizeof(s_mqtt_recv_data.recv_buffer));
    }
    print_mqtt_driver_log("mqtt_client_incoming_data_cb:reveiving incomming data.\n");
}

/*!
* @brief MQTT 接收到数据的回调函数
* 执行条件：MQTT 连接成功
*
* @param [in] : 用户提供的回调参数指针
* @param [in] : MQTT 收到数据的topic
* @param [in] : MQTT 收到数据的总长度
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
    s_mqtt_recv_data.recv_total = tot_len; //需要接收的总字节
    s_mqtt_recv_data.recv_len = 0;         //已接收字节计数归0

    //清空接收buffer
    memset(s_mqtt_recv_data.recv_buffer, 0, sizeof(s_mqtt_recv_data.recv_buffer));
}

/*!
* @brief MQTT 连接状态的回调函数
*
* @param [in] : MQTT 连接句柄
* @param [in] : 用户提供的回调参数指针
* @param [in] : MQTT 连接状态
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

        // 注册接收数据的回调函数
        mqtt_set_inpub_callback(client, mqtt_incoming_publish_cb, mqtt_incoming_data_cb, arg);

        //连接成功订阅主题
        mqtt_connect_success_process(client, arg);
    }
    else //没有连接成功，尝试重新连接
    {
        print_mqtt_driver_log("bsp_mqtt_connection_cb: Fail connected, status = %s-%d\n", lwip_strerr(status), status);
        mqtt_error_process_callback(client, arg);
    }
}

/*!
* @brief 连接到 mqtt 服务器
* 执行条件：无
*
* @param [in] : None
*
* @retval: 连接状态，如果返回不是 ERR_OK 则需要重新连接
*/
static err_t mqtt_connect_server(uint32_t times)
{
	ip_addr_t server_ip_addr;      /* 远端ip地址 */
	
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
	
    //进行连接,注意：如果需要带入 arg ，arg必须是全局变量，局部变量指针会被回收，大坑！！！！！
    ret = mqtt_client_connect(s_mqtt_client_handle, &server_ip_addr, MQTT_REMOTE_SERVER_PORT,\
                              mqtt_connection_cb, NULL, &mqtt_connect_info);
    /******************
	小提示：连接错误不需要做任何操作，mqtt_client_connect 中注册的回调函数里面做判断并进行对应的操作
	*****************/
//	print_mqtt_driver_log("bsp_mqtt_connect:ret_err:%d\r\n", ret);
    print_mqtt_driver_log("bsp_mqtt_connect: connect to mqtt %s-%d\n", lwip_strerr(ret), ret);
    return ret;
}





/*!
* @brief MQTT 发送数据的回调函数
* 执行条件：MQTT 连接成功
*
* @param [in] : 用户提供的回调参数指针
* @param [in] : MQTT 发送的结果：成功或者可能的错误
* @retval: None
*/
static void mqtt_client_pub_request_cb(void *arg, err_t result)
{
    //传进来的 arg 还原
    mqtt_client_t *client = (mqtt_client_t *)arg;

    if (result != ERR_OK)
    {
        print_mqtt_driver_log("mqtt_client_pub_request_cb: c002: Publish FAIL, result = %s\n", lwip_strerr(result));

        //错误处理
        mqtt_error_process_callback(client, arg);
    }
    else
    {
//        print_mqtt_driver_log("mqtt_client_pub_request_cb: c005: Publish complete!\n");
    }
}

/*!
* @brief 发送消息到服务器的接口函数
* 执行条件：无
*
* @param [in1] : mqtt 连接句柄
* @param [in2] : mqtt 发送 topic 指针
* @param [in3] : 发送数据包指针
* @param [in4] : 数据包长度
* @param [in5] : qos
* @param [in6] : retain
* @retval: 发送状态
* @note: 有可能发送不成功但是现实返回值是 0 ，需要判断回调函数 mqtt_client_pub_request_cb 是否 result == ERR_OK
*/
static err_t bsp_mqtt_publish(mqtt_client_t *client, char *pub_topic, char *pub_buf, uint16_t data_len, uint8_t qos, uint8_t retain)
{
    if ((client == NULL) || (pub_topic == NULL) || (pub_buf == NULL) || \
    	(data_len == 0) || (qos > 2) || (retain > 1))
    {
        print_mqtt_driver_log("bsp_mqtt_publish: input error@@");
        return ERR_VAL;
    }

    //判断是否连接状态
    if (mqtt_client_is_connected(client) != pdTRUE)
    {
        print_mqtt_driver_log("bsp_mqtt_publish: client is not connected\n");
        return ERR_CONN;
    }

    err_t err;
#ifdef USE_MQTT_MUTEX
    // 创建 mqtt 发送互斥锁
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
			/*发送失败是否主动断开连接*/
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
* @brief MQTT 订阅的回调函数
* 执行条件：MQTT 连接成功
*
* @param [in] : 用户提供的回调参数指针
* @param [in] : MQTT 订阅结果
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
        //错误处理
        mqtt_error_process_callback(client, arg);
    }
    else
    {
        print_mqtt_driver_log("bsp_mqtt_request_cb: sub SUCCESS!\n");
    }
}

/*!
* @brief mqtt 订阅
* 执行条件：连接成功
*
* @param [in1] : mqtt 连接句柄
* @param [in2] : mqtt 发送 topic 指针
* @param [in5] : qos
* @retval: 订阅状态
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

    //订阅及注册回调函数
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
* @brief 封装 MQTT 初始化接口
* 执行条件：无
*
* @retval: 无
*/
void mqtt_client_init(uint32_t times)
{
    print_mqtt_driver_log("Mqtt init...\n");

    // 连接服务器
    mqtt_connect_server(times);
}

void mqtt_disconnect_to_server(void)
{
	mqtt_disconnect(s_mqtt_client_handle);
}


#pragma pack()

