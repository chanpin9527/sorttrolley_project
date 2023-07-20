#include "app_rs485.h"
#include "data_manager.h"
#include "rs485_driver.h"
#include "system_tick.h"
#include "cmsis_os.h"
#include "stdio.h"
#include "string.h"
#include "usart.h"
#include "limit_driver.h"
#include "flash_parameter.h"
#include "iwdg_driver.h"

#include "main.h"

#pragma pack(1)

#define DEBUG_RS485_PROTOCOL  // printf
#ifdef DEBUG_RS485_PROTOCOL
#define print_rs485_log(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define print_rs485_log(fmt, ...)
#endif

#define DRUM_PARABOLA_LOCATION_THRESHOLD         (5)
#define DRUM_BACKWORD_TIMES_LIMIT                (400)
#define DRUM_BACKWORD_SEND_INTERVAL              (50)


enum
{
    DRUM_PARABOLA_CHECK = 0,
    DRUM_PARABOLA_THROW,
    DRUM_PARABOLA_JUDGE,
    DRUM_PARABOLA_BACK,
};

typedef struct
{
    uint8_t parabola_state;              //状态
	uint32_t backword_times;			 //回位次数统计
	uint32_t run_tick;			         //滚筒运行错误时间判断
	uint32_t send_tick;			         //滚筒回位发送间隔
}Drum_control_struct_type;



static Drum_control_struct_type s_drum_control_struct_arr[SLAVE_END] = {0};
//需要读取的数据
static Set_package_informtion_t s_package_information[SLAVE_END] = {0};
static Set_rail_operating_para_t s_rail_operating_para = {0};
static Report_realtime_operating_data_t s_car_run_data = {0};

static Data_manager_dev* s_package_info_dev = NULL;
static Data_manager_dev* s_operating_para_dev = NULL;
static Data_manager_dev *s_car_run_data_dev = NULL;
//需要设置的数据

static Drum_alarm_data_t s_drum_alarm_data = {0};
static Data_manager_dev* s_drum_alarm_data_dev = NULL;

extern SemaphoreHandle_t s_correct_zero_semp;



void init_app485_data_manager(void)
{
	s_drum_alarm_data_dev = create_data_manager("drum_alarm_data", sizeof(Drum_alarm_data_t), 0);
}

/**
 * @note      包裹位置标定
 * @param     *package_info 包裹信息
 * @param     *operating_para 配置参数
 * @return    1包裹成功,-1甩货失败
 */
static int32_t package_location_correct(Set_package_informtion_t *package_info,
                                        Set_rail_operating_para_t *operating_para,
                                        uint8_t roller_num, 
										uint32_t car_speed)
{
    if((package_info->trailer_number == 0) || (roller_num == 0)) //没有包裹||滚筒号为0
    {
        return 0;
    }
    package_info->gird_num += DEFAULT_HEAD_LENGTH + roller_num*DEFAULT_ROLLER_LENGTH - ((car_speed * 0.25) / 2);
    return 1;
}

/**
 * @note      滚筒甩货控制
 * @param     *package_info 包裹信息
 * @param     *drum_control 控制变量
 * @param     cur_location 当前位置
 * @param     roller_num 滚筒序号
 * @return    1包裹成功,-1甩货失败
 */
static int32_t drum_parabola_control(Set_package_informtion_t *package_info,
                                  Drum_control_struct_type *drum_control,
                                  uint32_t cur_location,
                                  uint8_t roller_num, 
								  Drum_alarm_data_t *drum_alarm_data)
{
    int32_t res = 0;
	uint32_t tick_cur = get_systick_time();
    uint8_t run_direction = package_info->parabola_direction;
    uint8_t parabola_state = drum_control->parabola_state;

    switch(parabola_state)
    {
        case DRUM_PARABOLA_CHECK:
            if((!!package_info->trailer_number) 
                && (cur_location >= package_info->gird_num) 
                && (cur_location <= package_info->gird_num+DRUM_PARABOLA_LOCATION_THRESHOLD))
            {
                parabola_state = DRUM_PARABOLA_THROW;
            }
            else if((!package_info->trailer_number) && (!read_limit_state(roller_num)))
            {
                drum_control->backword_times = 0;
                parabola_state = DRUM_PARABOLA_BACK;
            }
            break;
        case DRUM_PARABOLA_THROW:
            drum_run_forword_backword(roller_num, FOREWORD, run_direction);
            drum_control->run_tick = tick_cur;
            parabola_state = DRUM_PARABOLA_JUDGE;
            break;
        case DRUM_PARABOLA_JUDGE:
            if(tick_cur - drum_control->run_tick >= DRUM_RUN_TIME_MIN) //等待500ms
            {
                if(read_limit_state(roller_num))
                {
                    //滚筒未转，报警
                    res = -1;
					/*bit[14:0]分别代表15-1号滚筒的未转动报警*/
					drum_alarm_data->alarm_value |= (0x01 << (roller_num - 1));
                    parabola_state = DRUM_PARABOLA_CHECK;
                }
                else
                {
                    res = 1;
					drum_alarm_data->alarm_value &= ~(0x01 << (roller_num - 1));
                    parabola_state = DRUM_PARABOLA_BACK;
                }
            }
            break;
        case DRUM_PARABOLA_BACK:
            if(read_limit_state(roller_num))//滚筒已回位
            {
                parabola_state = DRUM_PARABOLA_CHECK;
                drum_control->backword_times = 0;
				drum_alarm_data->alarm_value &= ~(0x01 << (roller_num+15));
            }
            else
            {
                if(tick_cur - drum_control->send_tick >= DRUM_BACKWORD_SEND_INTERVAL)
                {
                    drum_control->send_tick = tick_cur;
                    if(drum_control->backword_times++ < DRUM_BACKWORD_TIMES_LIMIT)
                    {
                        drum_run_forword_backword(roller_num, BACKWORD, run_direction); //归位
                    }
                    else 
                    {
						/*bit[30:16]分别代表滚筒15-1的传感器缺失*/
						drum_alarm_data->alarm_value |= (0x01 << (roller_num+15));
                        //传感器缺失，报警
                    }
                }
            }
            break;
        default:
            break;
    }
    drum_control->parabola_state = parabola_state;
    return res;
}

void task_rs485_tx(void const *arg)
{
    int32_t res = 0;
    uint8_t i = 0;
    uint8_t parabola_direction = 0;
    uint8_t correct_zero_flag = 1;
	uint32_t tick_cur = 0, tick_pre = 0, tick_alive = 0;
    Set_package_informtion_t package_info = {0};
	osDelay(500);
    s_package_info_dev = find_data_manager("package_info");
    s_operating_para_dev = find_data_manager("operating_para");
    s_car_run_data_dev = find_data_manager("car_run_data");
    
	while(1)
	{
		tick_cur = get_systick_time();
		if(tick_cur - tick_alive >= (TASK_ALIVE_PRINT_PERIOD + 200))
		{
			tick_alive = tick_cur;
			print_rs485_log("\r\ntask rs485 is aliving!!!\r\n");
		}
		get_data_manager_data(s_package_info_dev, s_package_information, (sizeof(Set_package_informtion_t)<<4));  //获取滚筒参数
        get_data_manager_data(s_operating_para_dev, &s_rail_operating_para, sizeof(Set_rail_operating_para_t));  //获取小车配置参数
        get_data_manager_data(s_car_run_data_dev, &s_car_run_data, sizeof(Report_realtime_operating_data_t));  //获取上报参数
        if(xSemaphoreTake(s_correct_zero_semp, 0) == pdTRUE) //收到校零信号
//            xSemaphoreGiveFromISR(s_correct_zero_semp,&xHigherPriorityTaskWoken); //通知其他任务已校零
        {
            correct_zero_flag = 0;
        }
        if(!correct_zero_flag) //未经过校零，不能甩货
        {   
            for(i = SLAVE_01; i< SLAVE_END; i++)
            {     
                /*1.复制包裹信息*/
                memcpy(&package_info,&s_package_information[i],sizeof(Set_package_informtion_t));         
                /*2.位置标定*/
                package_location_correct(&package_info,&s_rail_operating_para,i,s_car_run_data.cur_speed);
                /*3.甩货控制*/
                res = drum_parabola_control(&package_info,&s_drum_control_struct_arr[i],s_car_run_data.cur_location,i, &s_drum_alarm_data);
                if(res == 1)//甩货成功，清除标志位
                {
                    parabola_direction = package_info.parabola_direction;
                    memset(&package_info,0x00,sizeof(Set_package_informtion_t)); //包裹信息清零
                    package_info.parabola_direction = parabola_direction;   //保留方向，回位时用到
                    get_data_manager_data(s_package_info_dev, s_package_information, (sizeof(Set_package_informtion_t)<<4));  //重新获取滚筒参数
                    memcpy(&s_package_information[i],&package_info,sizeof(Set_package_informtion_t));    
                    if(set_data_manager_data(s_package_info_dev, s_package_information, (sizeof(Set_package_informtion_t)<<4))!= (sizeof(Set_package_informtion_t)<<4))
                    {
                        print_rs485_log("data manager set s_package_info_dev error\n");
                    }
                    set_package_info_flash_param((const char *)&package_info,i);
                }
            }
			/*写入滚筒报警数据*/
			if(set_data_manager_data(s_drum_alarm_data_dev, &s_drum_alarm_data, sizeof(Drum_alarm_data_t)) != sizeof(Drum_alarm_data_t))
			{
				print_rs485_log("data manager set s_drum_alarm_data_dev error\n");
			}
			
        }
		tick_cur = get_systick_time();
		if(tick_cur - tick_pre >= 3000)
		{
			tick_pre = tick_cur;
			print_rs485_log("location:1: %d--2: %d--3: %d--4: %d--5: %d--6: %d--7: %d--8: %d--9: %d--10: %d--11: %d--12: %d--13: %d--14: %d--15: %d\r\n",\
			s_package_information[1].gird_num, s_package_information[2].gird_num, s_package_information[3].gird_num, \
			s_package_information[4].gird_num, s_package_information[5].gird_num, s_package_information[6].gird_num, \
			s_package_information[7].gird_num, s_package_information[8].gird_num, s_package_information[9].gird_num, \
			s_package_information[10].gird_num, s_package_information[11].gird_num, s_package_information[12].gird_num,\
			s_package_information[13].gird_num, s_package_information[14].gird_num, s_package_information[15].gird_num);
			
			for(i = SLAVE_01; i< SLAVE_END; i++)
			{
				if(((s_drum_alarm_data.alarm_value >> (i-1)) & 0x01) || ((s_drum_alarm_data.alarm_value >> (i+15)) & 0x01))
				{
					print_rs485_log("dru_alarm:%d:--no_ack：%d--no_back: %d\r\n",i, (!!(s_drum_alarm_data.alarm_value >> (i-1))), (!!(s_drum_alarm_data.alarm_value >> (i+15))));
				}
			}
		}
		osDelay(3);
	}
}

#pragma pack()
