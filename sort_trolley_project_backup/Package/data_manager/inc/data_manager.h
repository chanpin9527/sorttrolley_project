/*
 * description：data manager头文件，对外暴露接口，应用程序只需要包含这个头文件就可以
                无需包含data_manager_conf.h  data_manager_data_type.h
 * version: 1.0
 * Change Logs:
 * Date           Author           Notes
 * 2019-03-06     zhaojinlong      create
 * 2020-06-10     zhaojinlong      提炼出debug接口                            
 */

#ifndef __DATA_MANAGER_H__
#define __DATA_MANAGER_H__

#include "data_manager_conf.h"
#include "data_manager_data_type.h"
#include "cmsis_os.h"

#ifdef DEBUG_DATA_MANAGER
#define DATA_MANAGER_DEBUG_PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define DATA_MANAGER_DEBUG_PRINTF(fmt, ...)
#endif


/* 用于管理每个数据索引 */
typedef struct 
{
    char name[DATA_MANAGER_NAME_MAX]; // 数据名称
    osMutexId data_mutex;            // 数据保护互斥量
    uint32_t data_size;               // 原数大小
    uint32_t data_index;              // 在内存块中的索引
    uint8_t store_flag;               // 是否存储
}Data_manager_dev;

/* 不轻易使用，只是用来获得data manager内部状况的 */
typedef struct
{
    uint32_t total_size;
    uint32_t used_size;
    uint32_t left_size;
    uint32_t param_dev_total_size;
    uint32_t param_dev_used_size;
    uint32_t param_dev_left_size;
}Data_manager_info;

/**
 * @note    增加数据管理设备
 *          为什么不把data的指针直接传入那，因为那样的话，
 *          设置数据的模块，数据必须为全局变量，可能会引发不必要的麻烦
 * @param   name 储存的变量的名字索引
 * @param   data_size 要储存的数据大小，应该都在data_manager_data_type.h里面的数据
 * @param   store_flag 是否要存储这个数据
 * @return  Data_manager_dev* 返回data managere数据管理设备
 */
Data_manager_dev* create_data_manager(const char* name, uint32_t data_size, uint8_t store_flag);

/**
 * @note    用名字找到数据管理设备
 * @param   name 索引的名字
 * @return  Data_manager_dev* 返回data managere数据管理设备
 */
Data_manager_dev* find_data_manager(const char* name);

/**
 * @note    用索引找到数据管理设备
 * @param   index 索引
 * @return  Data_manager_dev* 返回data managere数据管理设备
 */
Data_manager_dev* find_data_manager_index(uint32_t index);

/**
 * @note    设置数据
 * @param   data_manager_dev 数据对应的数据管理设备
 * @param   data 要设置的数据指针
 * @return  设置成功返回数据大小
 *          设置失败返回 0
 */
uint32_t set_data_manager_data(Data_manager_dev* data_manager_dev , const void* data, uint32_t data_size);
//带地址偏移
uint32_t set_data_manager_offset_data(Data_manager_dev* data_manager_dev , const void* data, uint32_t data_size,uint32_t off_set);
/**
 * @note    获得数据
 * @param   data_manager_dev 数据对应的数据管理设备
 * @param   data 要获得的数据指针
 * @param   data_size 要获得的数据大小
 * @return  获得成功返回数据大小
 *          获得失败返回 0
 */
uint32_t get_data_manager_data(Data_manager_dev* data_manager_dev , void* data , uint32_t data_size);

/**
 * @note    获得data manager内部信息
 * @param   Data_manager_info data manager内部信息结构体指针
 * @return  返回 0
 */
uint32_t get_data_manager_info(Data_manager_info* info);

/**
 * @note    
 * @param   
 * @return  
 */
uint32_t set_data_manager_data_change_trigger(Data_manager_dev* data_manager_dev);


#endif
