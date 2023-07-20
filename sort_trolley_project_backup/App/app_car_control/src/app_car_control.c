#include "app_car_control.h"
#include "data_manager_data_type.h"
#include "data_manager.h"
#include "raster_driver.h"
#include "limit_driver.h"
#include "motor_driver.h"
#include "rs485_driver.h"
#include "lidar_driver.h"
#include "led_driver.h"
#include "app_diagnosis.h"
#include "system_tick.h"
#include "cmsis_os.h"
#include "stdio.h"
#include "string.h"
#include "main.h"
#include "easyflash.h"
#include "adc_driver.h"
#include "math.h"
#include "stdlib.h"
#include "app_rs485.h"
#include "flash_parameter.h"


#pragma pack(1) //内存字节对齐

#define DEBUG_CAR_CONTROL  // printf
#ifdef DEBUG_CAR_CONTROL
#define print_car_control_log(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define print_car_control_log(fmt, ...)
#endif

#define CAR_STATE_ALARM_TIMEOUT				   (10000)
#define RECORD_LOCATION_INTERVAL               (100)
#define CONTROL_TICK_INTERVAL                  (150)
#define FRE_PER_SPEED_BASE                     (50)
#define FRE_SPEED_ERROR                        (5)
#define FRE_PER_SPEED_MAX                      (3500)
#define fmax(a,b) (a>b?a:b)
#define fmin(a,b) (a<b?a:b)
#define fabs(a) ((a)<0?-(a):(a))

typedef enum 
{
    CAR_CONTROL_STATE_STANDY = 0,
    CAR_CONTROL_STATE_RUN,
    CAR_CONTROL_STATE_FORBID,
}Car_control_state_t;

typedef struct
{
	uint32_t cur_location;
	uint32_t car_speed;
}Car_realtime_run_data_t;

typedef struct
{
    uint32_t control_tick_pre;                 //上一次控制时间
    uint32_t dest_dac_value;                   //dac输出值
}Car_adjust_speed_data_t;

typedef struct
{
    Car_realtime_run_data_t realtime_run_data; //实时运行参数
    Set_start_stop_cmd_t start_stop_cmd;       //小车启停命令
    Car_adjust_speed_data_t adjust_speed_data; //调速数据 
    Car_control_state_t control_state;         //小车控制状态
    uint32_t location_pre;                     //上一次位置记录
	uint32_t start_tick;					   //记录启动时刻
}Car_control_data_t;

static Car_realtime_run_data_t s_car_realtime_run_data = {0};
static Car_control_data_t s_car_control_data = {0};

typedef struct
{
	void* buffer;
	uint16_t readCnt;
	uint16_t writeCnt;
	uint16_t size;
}Average_typedef;

#define SPEED_AVERAGE_INTERVAL    (250)
#define SPEED_AVERAGE_SIZE        (8)
static uint16_t s_speed_average_buffer[SPEED_AVERAGE_SIZE] = {0};
static Average_typedef s_speed_average_struct = {(void *)s_speed_average_buffer,0,0,SPEED_AVERAGE_SIZE};

typedef struct
{
	uint8_t flag;			//检测标志
	uint8_t cur_state;		//当前状态
	uint8_t pre_state;		//前一个状态
	uint32_t filter_tick; 	//滤波时间
	uint32_t timer;			//计时
	uint32_t counter;		//计数
}Raster_data_t;

static Raster_data_t s_count_raster = {0};
static Raster_data_t s_reset_raster = {0};

extern QueueHandle_t s_start_stop_cmd_msg_mq;
SemaphoreHandle_t s_correct_zero_semp = NULL;

//获取的数据
static Set_rail_operating_para_t s_rail_operating_para = {0};
static Lidar_strike_signal_t s_lidar_strike_signal = {0};
static Drum_alarm_data_t s_drum_alarm = {0};
static Record_operating_data_t s_record_data = {0};
//上报的数据
static Report_realtime_operating_data_t s_car_run_data = {0};
static Report_alarm_data_t s_car_alarm_data = {0};

static Data_manager_dev *s_lidar_strike_signal_dev = NULL;
static Data_manager_dev *s_drum_alarm_data_dev = NULL;
static Data_manager_dev* s_operating_para_dev = NULL;
static Data_manager_dev* s_record_data_dev = NULL;
static Data_manager_dev *s_car_run_data_dev = NULL;
static Data_manager_dev *s_car_alarm_dev = NULL;

#define ADC_DMA_BUFFER_SIZE            (16)
#define BATTERY_VOL_CHECK_PERIOD	   500
#define HEAD_BATTERY_VOL_THRESHOLD     (35)
#define DRUM_BATTERY_VOL_THRESHOLD     (39)

static uint16_t s_adc_dma_buffer[_ADC3_DEVICE_MAX][ADC_DMA_BUFFER_SIZE] = {0};
static float s_battery_vol[BATTERY_VOL_MAX_INDEX] = {0.0};

/**
 * @note      创建小车运行参数data_manager
 * @param     None
 * @return    None
 */
void init_set_data_manager(void)
{
	s_correct_zero_semp = xSemaphoreCreateCounting(1, 0);
    
	/*创建对应的数据管理*/
	s_car_run_data_dev = create_data_manager("car_run_data", sizeof(Report_realtime_operating_data_t), 0);
	s_car_alarm_dev = create_data_manager("car_alarm_data", sizeof(Report_alarm_data_t), 0);
}

/**
 * @note      设置小车实时运行数据
 * @param     *run_data 实时运行参数
 * @return    None
 */
static int32_t set_car_realtime_run_data(Car_realtime_run_data_t *run_data)
{
    memcpy(&s_car_realtime_run_data,run_data,sizeof(Car_realtime_run_data_t));
    return 0;
}

/**
 * @note      获取小车实时运行数据
 * @param     *run_data 实时运行参数
 * @return    None
 */
static int32_t get_car_realtime_run_data(Car_realtime_run_data_t *run_data)
{
    memcpy(run_data,&s_car_realtime_run_data,sizeof(Car_realtime_run_data_t));
    return 0;
}

/**
 * @note      设置光栅数据
 * @param     *count_raster_data 计数光栅数据
 * @param     *reset_raster_data 复位光栅数据
 * @return    None
 */
static int32_t set_raster_data(Raster_data_t *count_raster_data, Raster_data_t *reset_raster_data)
{
	if((count_raster_data == NULL) || (reset_raster_data == NULL))
		return -1;
	memcpy(&s_count_raster, count_raster_data, sizeof(Raster_data_t));
	memcpy(&s_reset_raster, reset_raster_data, sizeof(Raster_data_t));
	return 0;
}

/**
 * @note      获取光栅数据
 * @param     *count_raster_data 计数光栅数据
 * @param     *reset_raster_data 复位光栅数据
 * @return    None
 */
static int32_t get_raster_data(Raster_data_t *count_raster_data, Raster_data_t *reset_raster_data)
{
	if((count_raster_data == NULL) || (reset_raster_data == NULL))
		return -1;
	memcpy(count_raster_data, &s_count_raster, sizeof(Raster_data_t));
	memcpy(reset_raster_data, &s_reset_raster, sizeof(Raster_data_t));
	return 0;
}

/**
 * @note      接收小车的启停命令
 * @param     *start_stop 启停参数
 * @return    None
 */
static void handle_car_run_arg(Set_start_stop_cmd_t *start_stop)
{
	BaseType_t err;
    Set_start_stop_cmd_t start_stop_cmd = {0};
    err = xQueueReceive(s_start_stop_cmd_msg_mq, &start_stop_cmd, 0);
	if(pdTRUE == err)
    {
        memcpy(start_stop,&start_stop_cmd,sizeof(Set_start_stop_cmd_t));
    }
}

/**
 * @note      上报小车的运行参数
 * @param     *report_data 上报数据
 * @param     *control_data 小车控制数据
 * @param     *lidar_signal 雷达信号
 * @param     *battery_vol 电池电量
 * @return    None
 */
static int32_t report_car_run_state_arg(Report_realtime_operating_data_t *report_data,
                                     Car_control_data_t *control_data,
                                     Lidar_strike_signal_t *lidar_signal,
                                     float *battery_vol)
{
    int32_t delta_location = control_data->realtime_run_data.cur_location - control_data->location_pre;
	report_data->systick = get_systick_time();
	report_data->cur_location = control_data->realtime_run_data.cur_location;
	if(control_data->control_state == CAR_RUN)
	{
		report_data->cur_speed = control_data->realtime_run_data.car_speed;
	}
	else
	{
		report_data->cur_speed = 0;
	}
	report_data->lidar_state = lidar_signal->strike_signal;
	report_data->head_battery_vol = report_data->cur_speed*1000;//(uint32_t)(battery_vol[CAR_HEAD_BATTERY_VOL] * 1000);/*mv*/
	report_data->tail_battery_vol = control_data->adjust_speed_data.dest_dac_value*1000;//(uint32_t)(battery_vol[DRUM_BATTERY_VOL] * 1000);
    if(fabs(delta_location) >= RECORD_LOCATION_INTERVAL)
    {
        control_data->location_pre = control_data->realtime_run_data.cur_location;
        set_single_data_flash_param("cur_location",&(control_data->realtime_run_data.cur_location),sizeof(control_data->realtime_run_data.cur_location));
    }
    if(set_data_manager_data(s_car_run_data_dev, report_data, sizeof(Report_realtime_operating_data_t))!= sizeof(Report_realtime_operating_data_t))
	{
		print_car_control_log("data manager set s_car_run_data_dev error\n");
	}
    return 0;
}

/**
 * @note      获取小车的报警参数
 * @param     *alarm_data 报警数据
 * @param     *realtime_data 实时运行数据
 * @param     *lidar_signal 雷达信号
 * @param     *drum_alarm_data 滚筒报警信号
 * @param     *battery_vol 电池电量
 * @return    None
 */
static int32_t report_car_alarm_data(Report_alarm_data_t *alarm_data,
                                  Car_control_data_t *control_data,
                                  Lidar_strike_signal_t *lidar_signal,
                                  Drum_alarm_data_t *drum_alarm_data,
                                  float *battery_vol)
{
	uint16_t alarm_value = 0;
	uint32_t tick_cur = get_systick_time();
	
	if((control_data->control_state == CAR_RUN) && \
		(tick_cur - control_data->start_tick >= CAR_STATE_ALARM_TIMEOUT) && \
		(control_data->realtime_run_data.car_speed == 0)) 
	{
		alarm_data->car_run_state_alarm = 1;//小车运行状态报警
	}
	else
	{
		alarm_data->car_run_state_alarm = 0;
	}
	//车头电压监测
	if(battery_vol[CAR_HEAD_BATTERY_VOL] <= HEAD_BATTERY_VOL_THRESHOLD)
	{
		alarm_data->battery_vol_alarm |= 0x01 << CAR_HEAD_BATTERY_VOL;
	}
	else
	{
		alarm_data->battery_vol_alarm &= ~(0x01 << CAR_HEAD_BATTERY_VOL);
	}
	//车尾滚筒电压监测
	if(battery_vol[DRUM_BATTERY_VOL] <= DRUM_BATTERY_VOL_THRESHOLD)
	{
		alarm_data->battery_vol_alarm |= 0x01 << DRUM_BATTERY_VOL;
	}
	else
	{
		alarm_data->battery_vol_alarm &= ~(0x01 << DRUM_BATTERY_VOL);
	}
	
	//防撞预警
	alarm_data->collide_alarm = lidar_signal->strike_signal;
	//滚筒状态报警
	alarm_data->drum_alarm = drum_alarm_data->alarm_value;
	
	alarm_value = (((!!(alarm_data->car_run_state_alarm)) * CAR_STATE_ALARM) | \
					((!!(alarm_data->battery_vol_alarm >> CAR_HEAD_BATTERY_VOL)) * CAR_HEAD_BATTERY_ALARM) | \
					((!!(alarm_data->battery_vol_alarm >> DRUM_BATTERY_VOL)) * DRUM_BATTERY_ALARM));
	
	if(!(alarm_value))
	{
		RUN_LED_ON;
		FAULT_LED_OFF;
	}
	else
	{
		FAULT_LED_ON;
		RUN_LED_OFF;
	}
    if(set_data_manager_data(s_car_alarm_dev, alarm_data, sizeof(Report_alarm_data_t))!= sizeof(Report_alarm_data_t))
	{
		print_car_control_log("data manager set s_car_alarm_dev error\n");
	}
    return 0;
}

/**
 * @note      调节车速
 * @param     *adjust_speed_data 调速数据
 * @param     cur_speed    实际车速
 * @param     target_speed 目标车速
 * @return    目标控制值
 */
static int32_t adjust_car_speed(Car_adjust_speed_data_t *adjust_speed_data,
                                uint32_t cur_speed,
                                uint32_t target_speed)
{
    uint32_t dac_value = 0;
    uint32_t dest_dac_value = adjust_speed_data->dest_dac_value;
    uint32_t tick_cur = get_systick_time();
    int32_t delta_speed = target_speed - cur_speed;
    if(tick_cur - adjust_speed_data->control_tick_pre >= CONTROL_TICK_INTERVAL)
    {
        adjust_speed_data->control_tick_pre = tick_cur;
        if(delta_speed > 0)
        {
            delta_speed += FRE_SPEED_ERROR;
            dest_dac_value += fmin(delta_speed/FRE_PER_SPEED_BASE+1,FRE_PER_SPEED_MAX);
        }
        else 
        {
            delta_speed -= FRE_SPEED_ERROR;
            dest_dac_value += fmax(delta_speed/FRE_PER_SPEED_BASE-1,-FRE_PER_SPEED_MAX);
        }
        dac_value = fmax(1,fmin(dest_dac_value,FRE_PER_SPEED_MAX));
        set_motor_speed(dac_value);
        adjust_speed_data->dest_dac_value = dac_value;
//        print_car_control_log("dest_dac_value: %d\r\n",dac_value);
    }
    return adjust_speed_data->dest_dac_value;
}

/**
 * @note      小车速度滤波
 * @param     *average_struct 平均结构体
 * @param     speed_data 小车速度
 * @return    None
 */
static uint16_t car_speed_filter(Average_typedef *average_struct,uint16_t speed_data)
{
    uint16_t *buffer = (uint16_t *)average_struct->buffer ;
	uint32_t aver_sum = 0;
	uint8_t i=0;
	buffer[(average_struct->writeCnt++)] = speed_data;
	if(average_struct->writeCnt >= average_struct->size)
	{
		average_struct->writeCnt = 0;
	}
	for(i = 0;i<average_struct->size ;i++)
	{
		aver_sum += buffer[i];
	}
	return aver_sum/average_struct->size;
}

/**
 * @note      小车运行控制
 * @param     *control_data 小车控制数据
 * @param     *rail_operating_para 轨道配置参数
 * @param     *lidar_signal 雷达信号
 * @return    None
 */
static int32_t car_operating_control(Car_control_data_t *control_data,
                                     Set_rail_operating_para_t *rail_operating_para,
                                     Lidar_strike_signal_t *lidar_signal)
{
    Car_control_state_t control_state = control_data->control_state;
    uint32_t target_speed = 0;
    uint32_t cur_speed = control_data->realtime_run_data.car_speed*10;
    
    switch(control_state)
    {
        case CAR_CONTROL_STATE_STANDY:
            if(s_lidar_strike_signal.strike_signal == STRIKE_SIGNAL)
            {
                control_state = CAR_CONTROL_STATE_FORBID;
                print_car_control_log("car_control enter CAR_CONTROL_STATE_FORBID state\r\n");
            }
            else if(control_data->start_stop_cmd.start_stop_cmd == CAR_START)
            {
                control_data->adjust_speed_data.dest_dac_value = MOTOR_START_VOL;
                control_state = CAR_CONTROL_STATE_RUN;
				control_data->start_tick = get_systick_time();
                print_car_control_log("car_control enter CAR_CONTROL_STATE_RUN state\r\n");
            }
            else if(control_data->start_stop_cmd.start_stop_cmd == CAR_STOP)
            {
                motor_brake_on_off(ON);
            }
            break;
        case CAR_CONTROL_STATE_RUN:
            if(s_lidar_strike_signal.strike_signal == STRIKE_SIGNAL)
            {
                control_state = CAR_CONTROL_STATE_FORBID;
                print_car_control_log("car_control enter CAR_CONTROL_STATE_FORBID state\r\n");
            }
            else if(control_data->start_stop_cmd.start_stop_cmd == CAR_STOP)
            {
                control_state = CAR_CONTROL_STATE_STANDY;
                print_car_control_log("car_control enter CAR_CONTROL_STATE_STANDY state\r\n");
            }
            else if(control_data->start_stop_cmd.start_stop_cmd == CAR_START)
            {
                if(control_data->realtime_run_data.cur_location < rail_operating_para->package_desk_length)//处于供包台
                {
                    target_speed = rail_operating_para->enter_desk_speed*10;
                }
                else //不在供包台
                {
                    target_speed = rail_operating_para->normal_speed*10;
                }
                adjust_car_speed(&(control_data->adjust_speed_data),cur_speed,target_speed);//调节车速
            }
            else 
            {
                control_state = CAR_CONTROL_STATE_STANDY;
                print_car_control_log("car_control enter CAR_CONTROL_STATE_STANDY state\r\n");
            }
            break;
        case CAR_CONTROL_STATE_FORBID:
            motor_brake_on_off(ON);
            if(s_lidar_strike_signal.strike_signal == NO_STRIKE_SIGNAL)
            {
                control_state = CAR_CONTROL_STATE_STANDY;
                print_car_control_log("car_control enter CAR_CONTROL_STATE_STANDY state\r\n");
            }
            break;
        default:
            break;
    }
    control_data->control_state = control_state;
    return 0;
}


void task_car_control(void const *arg)
{	
	uint32_t tick_cur = 0, tick_pre = 0, tick_speed_pre = 0, tick_alive = 0;
    Car_realtime_run_data_t car_realtime_run_data = {0};
	rt_hw_adc_dma_init(_ADC2_DEVICE_INDEX, s_adc_dma_buffer[_ADC2_DEVICE_INDEX], ADC_DMA_BUFFER_SIZE);
	rt_hw_adc_dma_init(_ADC3_DEVICE_INDEX, s_adc_dma_buffer[_ADC3_DEVICE_INDEX], ADC_DMA_BUFFER_SIZE);
	
	osDelay(1000);
    s_operating_para_dev = find_data_manager("operating_para");
    s_lidar_strike_signal_dev = find_data_manager("lidar_strike_signal");
	s_drum_alarm_data_dev = find_data_manager("drum_alarm_data");
    s_record_data_dev = find_data_manager("record_data");
    get_data_manager_data(s_record_data_dev, &s_record_data, sizeof(Record_operating_data_t));  //获取实时运行参数
    s_car_control_data.realtime_run_data.cur_location = s_record_data.cur_location;//重启后获取记录位置
	set_car_realtime_run_data(&(s_car_control_data.realtime_run_data));
	while(1)
	{
		tick_cur = get_systick_time();
		if(tick_cur - tick_alive >= (TASK_ALIVE_PRINT_PERIOD + 100))
		{
			tick_alive = tick_cur;
			print_car_control_log("\r\ntask car control is aliving!!!\r\n");
		}
        get_data_manager_data(s_operating_para_dev, &s_rail_operating_para, sizeof(Set_rail_operating_para_t));  //获取小车配置参数
        get_data_manager_data(s_lidar_strike_signal_dev, &s_lidar_strike_signal, sizeof(Lidar_strike_signal_t)); //获取雷达数据
        get_data_manager_data(s_car_alarm_dev, &s_car_alarm_data, sizeof(Report_alarm_data_t)); //获取系统报警信息
        get_data_manager_data(s_drum_alarm_data_dev, &s_drum_alarm, sizeof(Drum_alarm_data_t)); //获取滚筒报警信息
		handle_car_run_arg(&(s_car_control_data.start_stop_cmd)); //处理接收的小车启停/速度命令
        /*0.小车速度和位置获取*/
        get_car_realtime_run_data(&car_realtime_run_data);
        s_car_control_data.realtime_run_data.cur_location = car_realtime_run_data.cur_location;
        if(tick_cur - tick_speed_pre >= SPEED_AVERAGE_INTERVAL)
        {
            tick_speed_pre = tick_cur;
            s_car_control_data.realtime_run_data.car_speed = car_speed_filter(&s_speed_average_struct,car_realtime_run_data.car_speed);
        }
        /*1.小车运行控制*/
		car_operating_control(&s_car_control_data,
                              &s_rail_operating_para,
                              &s_lidar_strike_signal); 
        /*2.小车状态上报*/
		report_car_run_state_arg(&s_car_run_data,
                                 &s_car_control_data,
                                 &s_lidar_strike_signal,
                                 s_battery_vol); 
        /*3.小车报警信息上报*/
		report_car_alarm_data(&s_car_alarm_data,
                              &s_car_control_data,
                              &s_lidar_strike_signal,
                              &s_drum_alarm,
                              s_battery_vol);

        /*4.采集电池电压*/
		if(tick_cur - tick_pre >= BATTERY_VOL_CHECK_PERIOD*5)
		{
			tick_pre = tick_cur;
//			print_car_control_log("car_speed:----------------%d-%d\r\n",s_rail_operating_para.enter_desk_speed, s_rail_operating_para.normal_speed);
			s_battery_vol[CAR_HEAD_BATTERY_VOL] = get_battery_voltage_value(_ADC3_DEVICE_INDEX, s_adc_dma_buffer[_ADC3_DEVICE_INDEX], ADC_DMA_BUFFER_SIZE);
			s_battery_vol[DRUM_BATTERY_VOL] = get_battery_voltage_value(_ADC2_DEVICE_INDEX, s_adc_dma_buffer[_ADC2_DEVICE_INDEX], ADC_DMA_BUFFER_SIZE);
		}
		osDelay(5);
	}
}

/**
 * @note      定时器中断处理函数
 * @param     None
 * @return    None
 */
void handle_tim_irq(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    Car_realtime_run_data_t real_run_data = {0};
	Raster_data_t count_raster = {0}, reset_raster = {0};
	uint32_t delta_raster = 0;
	
    get_car_realtime_run_data(&real_run_data);
	get_raster_data(&count_raster, &reset_raster);
	/*复位光栅检测*/
	if(reset_raster.flag == 1)
	{
		reset_raster.pre_state = read_reset_raster_statue();
		if(reset_raster.pre_state == RESET_RASTER_INITAL_LEVEL)
		{
			reset_raster.flag = 0;
			reset_raster.filter_tick = 0;
		}
	}
	reset_raster.cur_state = read_reset_raster_statue();
	if(reset_raster.flag == 0 && reset_raster.cur_state != reset_raster.pre_state)
	{
		reset_raster.filter_tick++;
		if(reset_raster.filter_tick >= FILTER_PERIOD)//软件滤波去毛刺（高电平持续1ms）
		{
			reset_raster.counter++;
			reset_raster.flag = 1;
			reset_raster.filter_tick = 0;
			if((reset_raster.counter >= RESET_RASTER_NUM) && (reset_raster.timer < RESET_RASTER_PERIOD))
			{
				real_run_data.cur_location = 0;
				reset_raster.counter = reset_raster.timer = 0;
                xSemaphoreGiveFromISR(s_correct_zero_semp,&xHigherPriorityTaskWoken); //通知其他任务已校零
                portYIELD_FROM_ISR(xHigherPriorityTaskWoken);//如果需要的话进行一次任务切换
			}
		}
	}
	
	if(reset_raster.counter)
	{
		reset_raster.timer++;
		if((reset_raster.timer > RESET_RASTER_PERIOD) && (reset_raster.counter < RESET_RASTER_NUM))
		{
			reset_raster.counter = reset_raster.timer = 0;
			reset_raster.flag = 1;
		}
	}
	/*计数光栅检测*/
	if(count_raster.flag == 1)
	{
		count_raster.pre_state = read_counter_raster_statue();
		if(count_raster.pre_state == COUNT_RASTER_INITAL_LEVEL)
		{
			count_raster.flag = 0;
			count_raster.filter_tick = 0;
		}
	}
	count_raster.cur_state = read_counter_raster_statue();
	if(count_raster.flag == 0 && count_raster.cur_state != count_raster.pre_state)
	{
		count_raster.filter_tick++;
		if(count_raster.filter_tick >= FILTER_PERIOD)//软件滤波去毛刺（低电平持续1ms）
		{
			real_run_data.cur_location++;
			count_raster.flag = 1;
			count_raster.filter_tick = 0;
			count_raster.counter++;
		}
	}
    count_raster.timer++;
    if((count_raster.counter >= RASTER_SPEED_PERIOD) || (count_raster.timer >= RASTER_SPEED_TIMEOUT))
    {
        delta_raster = count_raster.counter;
        if(count_raster.timer > 0)
        {
            real_run_data.car_speed = ((RASTER_GRID_LENGTH*delta_raster)*1000)/(count_raster.timer); //计算速度
        }
        if(real_run_data.car_speed < 5)
        {
           real_run_data.car_speed = 0; 
        }
        count_raster.timer = 0;
        count_raster.counter = 0;
    }
	
    set_car_realtime_run_data(&real_run_data);
	set_raster_data(&count_raster, &reset_raster);
}

#pragma pack()
