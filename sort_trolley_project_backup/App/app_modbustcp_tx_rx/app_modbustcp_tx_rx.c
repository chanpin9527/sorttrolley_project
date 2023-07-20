#include "app_modbustcp_tx_rx.h"
#include "mb.h"
#include "data_manager.h"
#include "data_manager_data_type.h"
#include "stdio.h"
#include "string.h"
#include "eth_udp_driver.h"
#include "rs485_driver.h"
#include "system_tick.h"
#include "cmsis_os.h"

#pragma pack(1)

#define DEBUG_MODBUSTCP_PROTOCOL  // printf
#ifdef DEBUG_MODBUSTCP_PROTOCOL
#define print_modbustcp_log(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define print_modbustcp_log(fmt, ...)
#endif

uint16_t usRegInputStart = REG_INPUT_START;
// ���ּĴ�������
uint16_t usRegHoldingBuf[REG_HOLDING_NREGS] = {[0] = 0, [47] = MOTOR_SPEED_NORMAL, [48] = MOTOR_SPEED_DESK, [49] = MOTOR_SPEED_NORMAL};
// ���ּĴ�����ʼ��ַ

/*****************************��ֹ����*******************************/
// ����Ĵ�������
uint16_t usRegInputBuf[REG_INPUT_NREGS] = {0};
// �Ĵ�����ʼ��ַ
uint16_t usRegHoldingStart = REG_HOLDING_START;
// ��Ȧ״̬
uint8_t ucRegCoilsBuf[REG_COILS_SIZE] = {0};

uint8_t ucRegDiscreteBuf[REG_DISCRETE_SIZE] = {0};
/*****************************��ֹ����*******************************/

/*���ݹ���*/
//��Ҫ����������
static Data_manager_dev *s_car_run_state_dev = NULL; 
static Data_manager_dev *s_car_alarm_dev = NULL;
static Data_manager_dev *s_SN_dev = NULL;
static Data_manager_dev *s_trailer_dev = NULL;

//��Ҫ���õ�����
static Data_manager_dev *s_car_start_stop_dev = NULL; 
static Data_manager_dev *s_drum_parabola_dev = NULL;
static Data_manager_dev *s_car_speed_dev = NULL;
static Data_manager_dev *s_drum_arg_dev = NULL;

/*��Ҫ��ȡ������*/
static uint8_t s_car_start_stop_cmd_buf[CAR_START_STOP_CMD_BUF_SIZE] = {0}; //С����ͣ����buf
static uint8_t s_drum_parabola_arg_buf[DRUM_PARABOLA_ARG_BUF_SIZE] = {0}; //�������buf
static uint8_t s_car_speed_arg_buf[CAR_SPEED_ARG_BUF_SIZE] = {0}; //С���ٶȲ���buf
static uint8_t s_drum_arg_buf[DRUM_ARG_BUF_SIZE] = {0}; //��Ͳ����buf

static Start_stop_cmd_t s_car_cmd = {0};//С����ͣ����
static Drum_parabola_arg_t s_drum_parabola_arg[SLAVE_END] = {0};//�������
static Car_speed_arg_t s_car_speed_arg = {0};//С���ٶȲ���
static Drum_arg_t s_drum_arg ={0};//��Ͳ����buf

/*��Ҫ���õ�����*/
static uint8_t s_car_run_state_buf[CAR_RUN_STATE_ARG_BUF_SIZE] = {0};//С��״̬
static uint8_t s_car_alarm_data_buf[CAR_ALARM_DATA_BUF_SIZE] = {0}; //����״̬
static uint8_t s_SN_buf[SN_BUF_SIZE] = {0}; //SN��
static uint8_t s_trailer_buf[TRAILER_PARCEL_BUF_SIZE] ={0}; //�ϳ��ں�

static Car_run_state_arg_t s_car_run_state_arg_struct = {0};
static Alarm_data_t s_car_alarm_data_struct = {0};
/*�����ݵ��ź���*/
SemaphoreHandle_t car_start_stop_semp = NULL;
SemaphoreHandle_t drum_parabola_arg_semp = NULL;
SemaphoreHandle_t car_speed_arg_semp = NULL;
SemaphoreHandle_t drum_arg_semp = NULL;
/*�������ź���*/
SemaphoreHandle_t car_start_stop_operate_semp = NULL;
SemaphoreHandle_t drum_parabola_arg_operate_semp = NULL;
SemaphoreHandle_t car_speed_arg_operate_semp = NULL;
SemaphoreHandle_t drum_arg_operate_semp = NULL;


/**********************************����С����ͣ\�������\С���ٶ�����\��Ͳ����************************************/

/**
 * @note      ��������̨�·���С����ͣ����
 * @param     None
 * @return    None
 */
static void handle_car_start_stop_cmd(void *arg)
{
	Start_stop_cmd_t *cmd = (Start_stop_cmd_t *)arg;
	
	cmd->start_stop_cmd = s_car_start_stop_cmd_buf[1];
}

/**
 * @note      ��������̨�·��Ĺ�Ͳ�������
 * @param     None
 * @return    None
 */
static void handle_drum_parabola_arg(void *arg)
{
	Drum_parabola_arg_t *parabola = (Drum_parabola_arg_t *)arg;
	Drum_parabola_arg_t temp_parabola = {0};
	
	temp_parabola.SN = s_drum_parabola_arg_buf[1] | ((uint16_t)s_drum_parabola_arg_buf[0]) << 8;
	temp_parabola.gird_num = s_drum_parabola_arg_buf[3] | ((uint16_t)s_drum_parabola_arg_buf[2]) << 8;
	temp_parabola.parabola_direction = s_drum_parabola_arg_buf[5] | ((uint16_t)s_drum_parabola_arg_buf[4]) << 8;
	temp_parabola.trailer_number = s_drum_parabola_arg_buf[7] | ((uint16_t)s_drum_parabola_arg_buf[6]) << 8;
	temp_parabola.package_desk_number = s_drum_parabola_arg_buf[9] | ((uint16_t)s_drum_parabola_arg_buf[8]) << 8;
	memcpy(&parabola[temp_parabola.trailer_number], &temp_parabola, sizeof(Drum_parabola_arg_t));
}



/**
 * @note      ��������̨�·���С������
 * @param     None
 * @return    None
 */
static void handle_car_speed_arg(void *arg)
{
	Car_speed_arg_t *speed = (Car_speed_arg_t *)arg;
	
	speed->car_normal_speed = s_car_speed_arg_buf[1] | ((uint16_t)s_car_speed_arg_buf[0]) << 8;
	speed->car_enter_desk_speed = s_car_speed_arg_buf[3] | ((uint16_t)s_car_speed_arg_buf[2]) << 8;
	speed->car_ready_paraola_speed = s_car_speed_arg_buf[5] | ((uint16_t)s_car_speed_arg_buf[4]) << 8;
}

/**
 * @note      ��������̨�·��Ĺ�Ͳ�����ٶȾ���
 * @param     None
 * @return    None
 */
static void handle_drum_arg(void *arg)
{
	Drum_arg_t *drum = (Drum_arg_t *)arg;
	
	drum->drum_parabola_speed = s_drum_arg_buf[1] | ((uint16_t)s_drum_arg_buf[0]) << 8;
	drum->drum_parabola_distance = s_drum_arg_buf[3] | ((uint16_t)s_drum_arg_buf[2]) << 8;
	drum->orbit_length = s_drum_arg_buf[5] | ((uint16_t)s_drum_arg_buf[4]) << 8;
}


/********************************��ȡ������С����ͣ\�������\С���ٶ�����\��Ͳ����****************************************/

/**
 * @note      ��ʼ�������ݵ��ź���
 * @param     None
 * @return    None
 */
void init_read_data_sem(void)
{
	car_start_stop_semp = xSemaphoreCreateCounting(1, 0);
	drum_parabola_arg_semp = xSemaphoreCreateCounting(1, 0);
	car_speed_arg_semp = xSemaphoreCreateCounting(1, 1);
	drum_arg_semp = xSemaphoreCreateCounting(1, 0);
}

/**
 * @note      ��ʼ��С���͹�Ͳ�������ź���
 * @param     None
 * @return    None
 */
void init_operate_sem(void)
{
	car_start_stop_operate_semp = xSemaphoreCreateCounting(1, 0);
	drum_parabola_arg_operate_semp = xSemaphoreCreateCounting(1, 0);
	car_speed_arg_operate_semp = xSemaphoreCreateCounting(1, 1);
	drum_arg_operate_semp = xSemaphoreCreateCounting(1, 0);
}

/**
 * @note      ���ö����ݵ��ź���
 * @param     None
 * @return    None
 */
void set_read_data_sem(uint16_t write_addr)
{
	uint16_t addr = 0;
	addr = write_addr;
	
//	if(addr == 0x104)
//		return ;
	
	if(addr == (uint16_t)CAR_START_STOP_CMD_ADDR)
	{
		xSemaphoreGive(car_start_stop_semp);
	}
	else if(addr == (uint16_t)DRUM_PARABOLA_ARG_ADDR)
	{
		xSemaphoreGive(drum_parabola_arg_semp);
	}
	else if(addr == (uint16_t)CAR_SPEED_ARG_ADDR)
	{
		xSemaphoreGive(car_speed_arg_semp);
	}
	else if(addr == (uint16_t)DRUM_ARG_ADDR)
	{
		xSemaphoreGive(drum_arg_semp);
	}
//	switch(write_addr) //���Ͷ�Ӧ���ź���
//	{
//		case CAR_START_STOP_CMD_ADDR: 
//			xSemaphoreGive(car_start_stop_semp); break;
//		case DRUM_PARABOLA_ARG_ADDR: 
//			xSemaphoreGive(drum_parabola_arg_semp); 
//		break;
//		case CAR_SPEED_ARG_ADDR: xSemaphoreGive(car_speed_arg_semp); break;
//		case DRUM_ARG_ADDR: xSemaphoreGive(drum_arg_semp); break;
//		default: break;
//	}
}

void init_read_data_manager(void)
{
	/*������Ӧ�����ݹ���*/
	s_car_start_stop_dev = create_data_manager("car_cmd_data", sizeof(Start_stop_cmd_t), 0);
	s_car_speed_dev = create_data_manager("car_speed_data", sizeof(Car_speed_arg_t), 0);
	s_drum_parabola_dev = create_data_manager("drum_parabola_data", sizeof(s_drum_parabola_arg), 0);
	s_drum_arg_dev = create_data_manager("drum_arg_data", sizeof(Drum_arg_t), 0);
}

/**
 * @note      ���ղ��������Ʋ���
 * @param     None
 * @return    None
 */
void eth_modbustcp_rx_data(void)
{
	//��С����ͣ
	if(xSemaphoreTake(car_start_stop_semp, 0) == pdTRUE)
	{
		eMBRegHoldingCB(s_car_start_stop_cmd_buf, CAR_START_STOP_CMD_ADDR, CAR_START_STOP_CMD_BUF_SIZE / 2, MB_REG_READ);
		
		handle_car_start_stop_cmd(&s_car_cmd);
		
		if(set_data_manager_data(s_car_start_stop_dev, &s_car_cmd, sizeof(Start_stop_cmd_t))!= sizeof(Start_stop_cmd_t))
		{
			print_modbustcp_log("data manager set s_car_start_stop_dev error\n");
		}
		xSemaphoreGive(car_start_stop_operate_semp); //ͬ��С������ͣ����
	}
	//���������
	if(xSemaphoreTake(drum_parabola_arg_semp, 0) == pdTRUE)
	{
		eMBRegHoldingCB(s_drum_parabola_arg_buf, DRUM_PARABOLA_ARG_ADDR, DRUM_PARABOLA_ARG_BUF_SIZE / 2, MB_REG_READ);
		
		handle_drum_parabola_arg(s_drum_parabola_arg);
		
		if(set_data_manager_data(s_drum_parabola_dev, s_drum_parabola_arg, sizeof(s_drum_parabola_arg))!= sizeof(s_drum_parabola_arg))
		{
			print_modbustcp_log("data manager set s_car_speed_dev error\n");
		}
		xSemaphoreGive(drum_parabola_arg_operate_semp); //ͬ����Ͳ�������
	}
	//��С���ٶ�
	if(xSemaphoreTake(car_speed_arg_semp, 0) == pdTRUE)
	{
		eMBRegHoldingCB(s_car_speed_arg_buf, CAR_SPEED_ARG_ADDR, CAR_SPEED_ARG_BUF_SIZE / 2, MB_REG_READ);
		
		handle_car_speed_arg(&s_car_speed_arg);
		
		if(set_data_manager_data(s_car_speed_dev, &s_car_speed_arg, sizeof(Car_speed_arg_t))!= sizeof(Car_speed_arg_t))
		{
			print_modbustcp_log("data manager set s_drum_parabola_dev error\n");
		}
		xSemaphoreGive(car_speed_arg_operate_semp);
	}
	//����Ͳ����
	if(xSemaphoreTake(drum_arg_semp, 0) == pdTRUE)
	{
		eMBRegHoldingCB(s_drum_arg_buf, DRUM_ARG_ADDR, DRUM_ARG_BUF_SIZE / 2, MB_REG_READ);
		
		handle_drum_arg(&s_drum_arg);
		
		if(set_data_manager_data(s_drum_arg_dev, &s_drum_arg, sizeof(Drum_arg_t))!= sizeof(Drum_arg_t))
		{
			print_modbustcp_log("data manager set s_drum_arg_dev error\n");
		}
		xSemaphoreGive(drum_arg_operate_semp);
	}
}
/******************************************����С����������*********************************************/

/**
 * @note      ��ȡ��8λ����
 * @param     num��Ҫ����������
 * @return    ����num�ĸ�8λ����
 */
static uint8_t get_msb8(uint16_t num)
{
	return (num >> 8) & 0xff;
}

/**
 * @note      ��ȡ��8λ����
 * @param     num��Ҫ����������
 * @return    ����num�ĵ�8λ����
 */
static uint8_t get_lsb8(uint16_t num)
{
	return (num & 0xff);
}

/**
 * @note      ����С�������в���
 * @param     run_buf��Ҫ���õ�buff
 * @return    None
 */
static void set_car_run_state_arg(uint8_t *run_buf)
{
	run_buf[1] = get_lsb8(s_car_run_state_arg_struct.cur_speed);
	run_buf[0] = get_msb8(s_car_run_state_arg_struct.cur_speed);
	
	run_buf[3] = get_lsb8(s_car_run_state_arg_struct.cur_location);
	run_buf[2] = get_msb8(s_car_run_state_arg_struct.cur_location);
	
	run_buf[5] = get_lsb8(s_car_run_state_arg_struct.run_time);
	run_buf[4] = get_msb8(s_car_run_state_arg_struct.run_time);
	
	run_buf[7] = get_lsb8(s_car_run_state_arg_struct.battery_vol);
	run_buf[6] = get_msb8(s_car_run_state_arg_struct.battery_vol);
}

/**
 * @note      ����С���ı�������
 * @param     alarm_buf��Ҫ���õ�buff
 * @return    None
 */
static void set_car_alarm_data(uint8_t *alarm_buf)
{
	alarm_buf[1] = get_lsb8(s_car_alarm_data_struct.car_run_state_alarm);
	alarm_buf[0] = get_msb8(s_car_alarm_data_struct.car_run_state_alarm);
	
	alarm_buf[3] = get_lsb8(s_car_alarm_data_struct.drum_alarm);
	alarm_buf[2] = get_msb8(s_car_alarm_data_struct.drum_alarm);
	
	alarm_buf[5] = get_lsb8(s_car_alarm_data_struct.battery_vol_alarm);
	alarm_buf[4] = get_msb8(s_car_alarm_data_struct.battery_vol_alarm);
	
	alarm_buf[7] = get_lsb8(s_car_alarm_data_struct.collide_alarm);
	alarm_buf[6] = get_msb8(s_car_alarm_data_struct.collide_alarm);
}


/**
 * @note      ����TCP���ݲ���
 * @param     None
 * @return    None
 */
static void eth_modbustcp_tx_data(void)
{
	s_car_run_state_dev = find_data_manager("car_run_data");
	s_car_alarm_dev = find_data_manager("car_alarm_data");
	s_SN_dev = find_data_manager("SN_data");
	s_trailer_dev = find_data_manager("trailer_data");
	
	if(s_car_run_state_dev != NULL) //��ȡ������С��״̬��Ϣ
	{
		get_data_manager_data(s_car_run_state_dev, &s_car_run_state_arg_struct, sizeof(Car_run_state_arg_t));
	}
	if(s_car_alarm_dev != NULL) //��ȡ�����ı�����Ϣ
	{
		get_data_manager_data(s_car_alarm_dev, &s_car_alarm_data_struct, sizeof(Alarm_data_t));
	}
	if(s_SN_dev != NULL) //��ȡ������SN��
	{
		get_data_manager_data(s_SN_dev, s_SN_buf, sizeof(s_SN_buf));
	}
	if(s_trailer_dev != NULL) //��ȡ���������ް�����Ϣ 
	{
		get_data_manager_data(s_trailer_dev, s_trailer_buf, sizeof(s_trailer_buf));
	}


	set_car_run_state_arg(s_car_run_state_buf);  //����С������״̬
	set_car_alarm_data(s_car_alarm_data_buf); //����С����������
	//sn�ź��ϳ��ں�ֱ�Ӵ��ݹ���������
	
	//����С������״̬����
	eMBRegHoldingCB(s_car_run_state_buf, CAR_RUN_STATE_ARG_ADDR, CAR_RUN_STATE_ARG_BUF_SIZE / 2, MB_REG_WRITE);
	//���ͱ�����Ϣ
	eMBRegHoldingCB(s_car_alarm_data_buf, CAR_ALARM_DATA_ADDR, CAR_ALARM_DATA_BUF_SIZE / 2, MB_REG_WRITE);
	//�����ϳ������ް���
	eMBRegHoldingCB(s_trailer_buf, TRAILER_PARCEL_ADDR, TRAILER_PARCEL_BUF_SIZE / 2, MB_REG_WRITE);
	//���Ͷ�Ӧ�İ�����SN��
	eMBRegHoldingCB(s_SN_buf, SN_ADDR, SN_BUF_SIZE / 2, MB_REG_WRITE);
}

void task_modbustcp_tx_rx(void const *arg)
{
	
	osDelay(3000); //�ȴ������ײ��ʼ�����
	
	eMBTCPInit(502); 
	eMBEnable();
	
	while(1)
	{			
		eMBPoll();
		/*���ݿ��Ʋ������¿���״̬*/
		eth_modbustcp_rx_data(); //���ղ��������Ʋ�������ͣ/�ٶ�/����/��Ͳ��
		/*����С����Ϣ*/
		eth_modbustcp_tx_data(); //����С��������״̬/����״̬/�ϳ�������Ϣ/
	}
}


#pragma pack()

