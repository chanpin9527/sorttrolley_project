/*
 * description：存放data manager管理的数据结构
 * version: 1.0
 * Change Logs:
 * Date           Author        Notes
 * 2019-03-06     kinloong      create
*/

#ifndef __DATA_MANAGER_DATA_TYPE_H__
#define __DATA_MANAGER_DATA_TYPE_H__

#include <stdint.h>
#include <stdbool.h>

#pragma pack(1) //内存字节对齐
enum
{
	SLAVE_START = 0x00,
	SLAVE_01,
	SLAVE_02,
	SLAVE_03,
	SLAVE_04,
	SLAVE_05,
	SLAVE_06,
	SLAVE_07,
	SLAVE_08,
	SLAVE_09,
	SLAVE_10,
	SLAVE_11,
	SLAVE_12,
	SLAVE_13,
	SLAVE_14,
	SLAVE_15,
	SLAVE_END,
};

/***********************package_info********************************/
#define SET_DEFAULT_FLASH_LOCK         (0x01)
#define PACKAGE_SN_SIZE                (20)
#define ROLLER_SIZE                    (SLAVE_END)

#define DEFAULT_HEAD_LENGTH                (25)
#define DEFAULT_ROLLER_LENGTH              (36)
#define DEFAULT_ORBIT_LENGTH               (15000)
#define DEFAULT_PACKAGE_DESK_LENGTH        (1800)
#define DEFAULT_NORMAL_SPEED               (160)
#define DEFAULT_ENTER_DESK_SPEED           (80)
#define DEFAULT_DRUM_PARABOLA_SPEED        (200)
#define DEFAULT_DRUM_PARABOLA_DISTANCE     (115)

#define TASK_ALIVE_PRINT_PERIOD					(10000)
#define	TASK_STATE_PRINT_PERIOD					(20000)


//ratser num两个光栅之间距离是2cm
#define RASTER_GRID_LENGTH				(2)
#define RASTER_SPEED_PERIOD				(25) 		//根据光栅计算速度的周期
#define RASTER_SPEED_TIMEOUT    		(5000) 		//根据光栅计算速度的周期-超时
#define RESET_RASTER_NUM				(2)
#define RESET_RASTER_PERIOD				(1500)


#define RASTER_GRID_DESK_AREA_MIN		(0)		//供包区域的光栅上下限
#define RASTER_GRID_DESK_AREA_MAX		(1500)

#define RASTER_GRID_NORMAL_AREA_MIN		(1500)			//正常运行区域光栅上下限
#define RASTER_GRID_NORMAL_AREA_MAX		(15000)

#define DESK_LOCATION					(850)
#define CAR_SPEED_MONITOR_PERIOD		(1000)
#define CAR_SPEED_ERR_PERIOD			(2000)



//速度电压
#define MOTOR_START_VOL			500 //启动电压
#define MOTOR_SPEED_DESK		75
#define MOTOR_SPEED_NORMAL		200
#define MOTOR_SPEED_MIN			(MOTOR_SPEED_DESK - 2)
#define MOTOR_SPEED_MAX			(MOTOR_SPEED_NORMAL + 10)	

#define ADD_SPEED_PERIOD		350 //加速周期
#define SUB_SPEED_PERIOD		6000
#define ADD_SPEED				50
#define ADD_START_SPEED			30

#define SPEED_DELTA				48//56
#define SPEED_VOL_MAX_SHIFT		3
#define SPEED_VOL_K				(12)
#define SPEED_VOL_B				(180)
#define SPEED_VOL_CALCULATE(v)	((SPEED_VOL_K * v) + SPEED_VOL_B)

//速度计算方式
#define FILTER_PERIOD	1//滤波周期（多少个tim中断时间）


//光栅位置计算方式
#define RASTER_COUNT_MODE_INT	(0)
#define RASTER_COUNT_MODE_TIM	(!RASTER_COUNT_MODE_INT)
#define RESET_RASTER_EN			1

//光栅未触发时的电平状态
#define COUNT_RASTER_INITAL_LEVEL	(1)
#define RESET_RASTER_INITAL_LEVEL	(0)

#define UDP_DATA_SIZE			1206
#define END_FLAG_NUM			1
#define LIDAR_UDP_DATA_SIZE		(UDP_DATA_SIZE+END_FLAG_NUM)

//雷达数据防撞范围 (mm单位) 直线
#define LIDAR_DATA_MIN	20
#define LIDAR_DATA_MAX	6000

#define LIDAR_RESULOTION	(10)
#define LEFT_LIDAR_FRAME_HEAD	(uint16_t)(0x4b00)
#define MIDDLE_LIDAR_FRAME_HEAD	(uint16_t)(0x3c00)
#define RIGHT_LIDAR_FRAME_HEAD	(uint16_t)(0x2d00)


typedef enum
{
	DEFAULT_ONE 				= 0X00,
	SET_START_STOP 				= 0x01,		//设置启停命令消息-down
	SET_PACKAGE_INFORMATION 	= 0x02,		//设置包裹信息-down
	SET_RAIL_OPERATING_PARA 	= 0x03,		//设置设置小车运行参数-down
	GET_OPERATING_PARA 			= 0x10,		//请求小车运行参数 - up
	REPORT_REALTIME_DATA 		= 0x20,		//上报实时运行数据-up
	REPORT_PACKAGE_INFORMATION 	= 0x21,		//上报包裹信息-up
	REPORT_ALARM_DATA 			= 0x22,		//上报报警信息-up
}Auto_distribute_msg_type;



//启停命令
enum
{
	CAR_STOP = 0x00,
	CAR_START,
	CAR_INVALID
};

//小车状态
enum
{
	CAR_STANBY 	= 0, //小车待机状态
	CAR_RUN 	= 1, //小车运行状态
	CAR_FORBID 	= 2  //小车禁止启动
};

//雷达数据
enum
{
	LEFT_LIDAR_DATA_INDEX = 0,
	MIDDLE_LIDAR_DATA_INDEX = 1,
	RIGHT_LIDAR_DATA_INDEX = 2,
	LIDAR_DATA_INDEX_MAX = 3
};

enum
{
	LIDAR_ANGLE_ARE_MIN_INDEX = 0,
	LIDAR_ANGLE_ARE_MAX_INDEX = 1,
};

enum LIDAR_SIGNAL
{
	NO_STRIKE_SIGNAL = 0,
	STRIKE_SIGNAL = 1,
};
/*需要接收的数据*/
typedef struct
{
	uint16_t start_stop_cmd;
	uint16_t padding;
}Set_start_stop_cmd_t;

typedef struct
{
	uint8_t sn[PACKAGE_SN_SIZE];
}Package_sn_t;

typedef struct
{
	Package_sn_t SN;				//SN号
	uint32_t gird_num;				//目标位置
	uint8_t parabola_direction;		//抛物方向
	uint8_t trailer_number;			//滚筒编号（1-15）
	uint8_t padding[2];
}Set_package_informtion_t;

typedef struct
{
	uint32_t orbit_length; 				//轨道总长度 单位：栅格（间隔1cm）
	uint32_t package_desk_length; 		//供包台长度 单位：栅格（间隔1cm）
	uint16_t normal_speed; 			    //正常运行速度 单位：cm/s
	uint16_t enter_desk_speed; 		    //供包台运行速度 单位：cm/s
	uint16_t drum_parabola_speed; 		//滚筒抛件速度 单位：cm/s
	uint16_t drum_parabola_distance; 	//滚筒抛件距离 单位：cm
	uint8_t padding[4];
}Set_rail_operating_para_t;

/*需要发送的数据*/
//连接server之后请求数据
typedef struct
{
	uint8_t padding[4];
}Get_operating_para_t;

//上报车辆相关信息
typedef struct 
{
	uint32_t systick; 			//mcu运行时间 单位：ms
	uint32_t cur_location; 		//当前位置 单位：栅格（间隔1cm）
	uint16_t cur_speed; 		//当前车速 单位：cm/s
	uint16_t lidar_state; 		//激光雷达状态 0-无障碍物 1-有障碍物
	uint32_t head_battery_vol; 	//车头电池电压 单位：mv
	uint32_t tail_battery_vol; 	//车尾电池电压 单位：mv
	uint8_t padding[4];
}Report_realtime_operating_data_t;

//上报包裹的SN号
typedef struct
{
	uint16_t bits_sn;
	uint16_t padding;
	//Package_sn_t roller_sn[0];
}Report_package_information_t;

//上报报警信息
typedef struct 
{
	uint32_t car_run_state_alarm; 	//运行状态报警
	uint32_t drum_alarm; 			//滚筒报警
	uint8_t battery_vol_alarm; 	    //电池电压报警
	uint8_t collide_alarm; 		    //碰撞报警
	uint8_t reserved[2]; 			//预留
}Report_alarm_data_t;

enum
{
	NOTIFY_DEFAULT 			= 0,
	NOTIFY_CAR_ST_CMD 		= 1,
	NOTIFY_CAR_RAIL_PARA 	= 2,
	NOTIFY_485_PACKAGE_MSG 	= 1,
	NOTIFY_485_RAIL_PARA 	= 2,
	NOTIFY_485_RST_RASTER	= 3,
};


//雷达数据撞击信号
typedef struct
{
	uint8_t strike_signal;
}Lidar_strike_signal_t;

typedef struct
{
	uint16_t area_id;
	uint8_t rail_id1;
	uint8_t rail_id2;
	uint16_t car_id;
}Car_ip_t;

//上报车辆相关信息
typedef struct 
{
	uint32_t cur_location; 		//当前位置 单位：栅格（间隔1cm）
    uint32_t connect_times;     //mqtt连接次数，每次连接递增1
}Record_operating_data_t;


/*设备所有参数*/
typedef struct
{
    Set_package_informtion_t  package_info[ROLLER_SIZE];     // 包裹参数
    Set_rail_operating_para_t rail_operating_para;           // 轨道配置参数
    Record_operating_data_t record_data;           // 记录运行数据
}All_flash_param_st;


#pragma pack() //内存字节对齐

#endif
