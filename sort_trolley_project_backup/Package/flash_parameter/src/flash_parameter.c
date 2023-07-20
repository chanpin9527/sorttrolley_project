/*
 * description：flash_parameter用于所有参数的读写
 * version: 1.0
 * Change Logs:
 * Date           Author           Notes
 * 2023-04-15     litao            create                       
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "cmsis_os.h"
#include "easyflash.h"
#include "flash_parameter.h"
#include "data_manager.h"
#include "system_tick.h"
#include "iwdg_driver.h"

#pragma pack(1) //内存字节对齐

#define DEBUG_FLASH_TASK  // printf
#ifdef DEBUG_FLASH_TASK
#define print_flash_task_log(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define print_flash_task_log(fmt, ...)
#endif

static Data_manager_dev* package_info_dev = NULL;
static Data_manager_dev* operating_para_dev = NULL;
static Data_manager_dev* s_record_data_dev = NULL;

static All_flash_param_st s_all_flash_param = {0};


/*
 * @note    创建所有参数管理描述符
 * @param   None
 * @return   
 */
int32_t creat_flash_manager_dev(void)
{
	int32_t ret=0;
	
     /* 创建数据描述符 */
    package_info_dev = create_data_manager("package_info", (sizeof(Set_package_informtion_t)<<4), 0);
	if (package_info_dev == NULL)
	{
		print_flash_task_log("data manager create package_info_dev error\r\n");		
        ret = -1;		
	}
    operating_para_dev = create_data_manager("operating_para", sizeof(Set_rail_operating_para_t), 0);
    if (operating_para_dev == NULL)
    {
        print_flash_task_log("data manager create operating_para_dev error\r\n");		
        ret = -1;		
	}
    s_record_data_dev = create_data_manager("record_data", sizeof(Record_operating_data_t), 0);
    if (s_record_data_dev == NULL)
    {
        print_flash_task_log("data manager create s_record_data_dev error\r\n");		
        ret = -1;		
	}
    return ret;	
}

/*
 * @note    读取flash数据里的包裹相关参数
 * @param   None
 * @return   
 */
static int32_t get_package_info_flash_param(Set_package_informtion_t *env_para)
{
	int32_t ret = 0;
    uint8_t i=0;
    uint32_t data_len = 0;
    char str_number[2] = {0};
    char package_info[14] = "package_info";
    for(i=SLAVE_01;i<SLAVE_END;i++)
    {
        iwdg_feeddog();
        sprintf(str_number,"%02d",i);
        strcat(package_info,str_number);
        data_len = ef_get_env_blob(package_info,&(env_para[i]) ,sizeof(Set_package_informtion_t ),NULL);
//        print_flash_task_log("%s get data len: %d\r\n",package_info,data_len);
        package_info[12] = 0;
        package_info[13] = 0;
    }

	if(set_data_manager_data(package_info_dev,env_para ,(sizeof(Set_package_informtion_t)<<4))!=(sizeof(Set_package_informtion_t)<<4))
	{
	    print_flash_task_log("data manager set package_info_dev error\n");
	    ret = -1;
	}
    return ret;	
}	

/*
 * @note    读取flash数据里的配置参数
 * @param   None
 * @return   
 */
static int32_t get_operating_para_flash_param(Set_rail_operating_para_t *env_para)
{
	int32_t ret = 0;
    
    ef_get_env_blob("orbit_length",&(env_para->orbit_length) ,sizeof(env_para->orbit_length),NULL);
    ef_get_env_blob("package_desk_length",&(env_para->package_desk_length) ,sizeof(env_para->package_desk_length),NULL);
    ef_get_env_blob("normal_speed",&(env_para->normal_speed) ,sizeof(env_para->normal_speed),NULL);
    ef_get_env_blob("enter_desk_speed",&(env_para->enter_desk_speed) ,sizeof(env_para->enter_desk_speed),NULL);
    ef_get_env_blob("drum_parabola_speed",&(env_para->drum_parabola_speed) ,sizeof(env_para->drum_parabola_speed),NULL);
    ef_get_env_blob("drum_parabola_distance",&(env_para->drum_parabola_distance) ,sizeof(env_para->drum_parabola_distance),NULL);
    
	if(set_data_manager_data(operating_para_dev,env_para ,sizeof(Set_rail_operating_para_t))!=sizeof(Set_rail_operating_para_t))
	{
	    print_flash_task_log("data manager set operating_para_dev error\n");
	    ret = -1;
	}
    return ret;	
}	

/*
 * @note    读取flash数据里的运行数据
 * @param   None
 * @return   
 */
static int32_t get_realtime_data_flash_param(Record_operating_data_t *env_para)
{
	int32_t ret = 0;
    
    ef_get_env_blob("cur_location",&(env_para->cur_location) ,sizeof(env_para->cur_location),NULL);
    ef_get_env_blob("connect_times",&(env_para->connect_times) ,sizeof(env_para->connect_times),NULL);
    
	if(set_data_manager_data(s_record_data_dev,env_para ,sizeof(Record_operating_data_t))!=sizeof(Record_operating_data_t))
	{
	    print_flash_task_log("data manager set s_record_data_dev error\n");
	    ret = -1;
	}
    return ret;	
}

/*
 * @note    将flash读出所有数据
 * @param   None
 * @return   
 */
static int32_t init_flash_get_all_data(All_flash_param_st *all_flash)
{
	int32_t ret = 0;
	    
    iwdg_feeddog();
    ret = get_package_info_flash_param(all_flash->package_info);
    iwdg_feeddog();
    ret = get_operating_para_flash_param(&(all_flash->rail_operating_para));
    iwdg_feeddog();
    ret = get_realtime_data_flash_param(&(all_flash->record_data));
    iwdg_feeddog();
	
    return ret;
}

/*
 * @note    flash失败时，使用默认值
 * @param   None
 * @return   
 */
static int32_t flash_use_default_value(void)
{
	int32_t ret=0;
	extern  All_flash_param_st default_config_para;
	memcpy (&s_all_flash_param,&default_config_para,sizeof(All_flash_param_st));
	
	if(set_data_manager_data(package_info_dev,s_all_flash_param.package_info ,(sizeof(Set_package_informtion_t)<<4))!=(sizeof(Set_package_informtion_t)<<4))
	{
	    print_flash_task_log("data manager set package_info_dev error\n");
        ret = -1;
	}	
	if(set_data_manager_data(operating_para_dev,&s_all_flash_param.rail_operating_para ,sizeof(Set_rail_operating_para_t))!=sizeof(Set_rail_operating_para_t))
	{
	   print_flash_task_log("data manager set package_info_dev error\n");
	   ret = -1;
	}
	if(set_data_manager_data(s_record_data_dev,&s_all_flash_param.record_data ,sizeof(Record_operating_data_t))!=sizeof(Record_operating_data_t))
	{
	   print_flash_task_log("data manager set s_record_data_dev error\n");
	   ret = -1;
	}

	return ret;
}

/*
 * @note    初始化应用参数
 * @param   status spiflash是否ok
 * @return   
 */
int32_t init_app_flash(int32_t flash_status)
{
	int32_t res=0;
	uint8_t lock_temp =0;
	
	/* 1. flash无效则用默认参数*/
    iwdg_feeddog();
    if(flash_status != EF_NO_ERR)
    {
        flash_use_default_value();
        print_flash_task_log("use defalut value,flash status: %d\r\n",flash_status);
		return 0;
    }
    else 
    {
        print_flash_task_log("flash init success!\r\n");
    }
	
	/*2.判断是否需要重置默认值,条件后续完善*/
    iwdg_feeddog();
    res=ef_get_env_blob("set_default_ver",&lock_temp,sizeof(lock_temp ),NULL);	
    if(lock_temp != SET_DEFAULT_FLASH_LOCK)
    {
        print_flash_task_log("set_default_ver is not same ,will set_default_ver!!!!");
        if(ef_env_set_default()!=EF_NO_ERR)  //恢复出厂值
		{
		   print_flash_task_log("ef_env_set_default error!\n");
		}
		else
		{
		   print_flash_task_log("ef_env_set_default success!\n");
		};
    }
    
	/* 3.获取所有参数 */
	res = init_flash_get_all_data(&s_all_flash_param);
    if(res == 0)
    {
        print_flash_task_log("init_flash_get_all_data success!\n");
    }
    return res;
}

/*
 * @note    设置flash数据里的包裹信息
 * @param   *env_para 包裹参数
 * @param   number 滚筒号
 * @return   
 */
int32_t set_package_info_flash_param(const char *env_para,uint8_t number)
{
	EfErrCode res;
    char str_number[2] = {0};
    char package_buf[14] = "package_info";
    Set_package_informtion_t * package_info = (Set_package_informtion_t *)env_para;
    
    sprintf(str_number,"%02d",number);
    strcat(package_buf,str_number);
    res = ef_set_env_blob(package_buf,package_info ,sizeof(Set_package_informtion_t));
    if(res!=EF_NO_ERR)
    {
        print_flash_task_log(" storage %s error!\r\n",package_buf);
    }
    return 0;	
}

/*
 * @note    设置flash数据里的配置参数
 * @param   None
 * @return   
 */
int32_t set_operating_para_flash_param(const char *env_para)
{
	EfErrCode res;
    Set_rail_operating_para_t *operating_para = (Set_rail_operating_para_t *)env_para;
    
    res = ef_set_env_blob("orbit_length",&(operating_para->orbit_length) ,sizeof(operating_para->orbit_length));
    if(res!=EF_NO_ERR)
    {
        print_flash_task_log(" storage orbit_length error!\r\n");
    }
    res = ef_set_env_blob("package_desk_length",&(operating_para->package_desk_length) ,sizeof(operating_para->package_desk_length));
    if(res!=EF_NO_ERR)
    {
        print_flash_task_log(" storage package_desk_length error!\r\n");
    }
    res = ef_set_env_blob("normal_speed",&(operating_para->normal_speed) ,sizeof(operating_para->normal_speed));
    if(res!=EF_NO_ERR)
    {
        print_flash_task_log(" storage normal_speed error!\r\n");
    }
    res = ef_set_env_blob("enter_desk_speed",&(operating_para->enter_desk_speed) ,sizeof(operating_para->enter_desk_speed));
    if(res!=EF_NO_ERR)
    {
        print_flash_task_log(" storage enter_desk_speed error!\r\n");
    }
    res = ef_set_env_blob("drum_parabola_speed",&(operating_para->drum_parabola_speed) ,sizeof(operating_para->drum_parabola_speed));
    if(res!=EF_NO_ERR)
    {
        print_flash_task_log(" storage drum_parabola_speed error!\r\n");
    }
    res = ef_set_env_blob("drum_parabola_distance",&(operating_para->drum_parabola_distance) ,sizeof(operating_para->drum_parabola_distance));
    if(res!=EF_NO_ERR)
    {
        print_flash_task_log(" storage drum_parabola_distance error!\r\n");
    }
    
    return 0;	
}


/*
 * @note    设置flash单个运行数据
 * @param   None
 * @return   
 */
int32_t set_single_data_flash_param(const char *key, const void *value_buf, size_t buf_len)
{
	EfErrCode res;
    
    res = ef_set_env_blob(key,value_buf ,buf_len);
    if(res!=EF_NO_ERR)
    {
        print_flash_task_log(" storage %s error!\r\n",key);
    }
    
    return 0;	
}


#pragma pack()


