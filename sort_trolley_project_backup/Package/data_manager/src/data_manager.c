/*
 * description：data manager核心文件，用于管理系统数据
 * version: 1.0
 * Change Logs:
 * Date           Author           Notes
 * 2019-03-06     zhaojinlong      create
 * 2020-06-10     zhaojinlong      提炼出debug接口，增加注释                          
 */

#include <stdint.h>
#include <string.h>
#include "data_manager.h"

static uint32_t s_data_manger_index = 0;          // 已使用的data_manager数量
static uint32_t s_data_manger_pool_index = 0;     // 已使用的data_manager pool数量
static uint8_t s_data_manager_pool[DATA_MANAGER_MAX_DATA_SIZE] = {0};
static Data_manager_dev s_data_manager_list[DATA_MANAGER_MAX_DEV_NUM];

/**
 * @note    增加数据管理设备
 *          为什么不把data的指针直接传入那，因为那样的话，
 *          设置数据的模块，数据必须为全局变量，可能会引发不必要的麻烦
 * @param   name 储存的变量的名字索引
 * @param   data_size 要储存的数据大小，应该都在data_manager_data_type.h里面的数据
 * @param   store_flag 是否要存储这个数据
 * @return  Data_manager_dev* 返回data managere数据管理设备
 */
Data_manager_dev* create_data_manager(const char* name, uint32_t data_size, uint8_t store_flag)
{
    Data_manager_dev* temp_data_dev = NULL;
    if (name == NULL)
        return NULL;
    if (data_size == 0)
        return NULL;
    if (s_data_manger_index >= DATA_MANAGER_MAX_DEV_NUM)
        return NULL;
    if (s_data_manger_pool_index + data_size >= DATA_MANAGER_MAX_DATA_SIZE)
        return NULL;
    
    temp_data_dev = find_data_manager(name);
    /* 如果索引名字相同则返回之前的管理结构，可能会有坑 */
    if(temp_data_dev != NULL)
    {
    	return temp_data_dev;
    }
    osMutexDef(myMutex01);
	s_data_manager_list[s_data_manger_index].data_mutex = osMutexCreate(osMutex(myMutex01));//xSemaphoreCreateMutex();
    
    strncpy(s_data_manager_list[s_data_manger_index].name , name , DATA_MANAGER_NAME_MAX);
    s_data_manager_list[s_data_manger_index].name[DATA_MANAGER_NAME_MAX - 1] = 0; // 防止没有结束符
    s_data_manager_list[s_data_manger_index].data_size = data_size;
    s_data_manager_list[s_data_manger_index].data_index = s_data_manger_pool_index;
    s_data_manager_list[s_data_manger_index].store_flag = store_flag;
    s_data_manger_index ++;
    s_data_manger_pool_index += data_size;

    return &(s_data_manager_list[s_data_manger_index - 1]);
}

/**
 * @note    用名字找到数据管理设备
 * @param   name 索引的名字
 * @return  Data_manager_dev* 返回data managere数据管理设备
 */
Data_manager_dev* find_data_manager(const char* name)
{
    uint32_t i = 0;
    if (name == NULL)
        return NULL;
    for (i = 0; i < s_data_manger_index ; i++)
    {
        if (strcmp(name , s_data_manager_list[i].name) == 0)
            return &(s_data_manager_list[i]);
    }
    
    return NULL;
}

/**
 * @note    用索引找到数据管理设备
 * @param   index 索引
 * @return  Data_manager_dev* 返回data managere数据管理设备
 */
Data_manager_dev* find_data_manager_index(uint32_t index)
{
    if (index >= DATA_MANAGER_MAX_DEV_NUM)
        return NULL;
    return &(s_data_manager_list[index]);
}

/**
 * @note    设置数据，数据改变了才进行赋值
 *          有个弊端是有可能会增加时长
 * @param   data_manager_dev 数据对应的数据管理设备
 * @param   data 要设置的数据指针
 * @return  设置成功返回数据大小
 *          设置失败返回 0
 */
uint32_t set_data_manager_data(Data_manager_dev* data_manager_dev , const void* data, uint32_t data_size)
{
    int32_t res;
    if (data_manager_dev == NULL)
        return 0;
    if (data_manager_dev->data_size != data_size)
        return 0;
    
    /* 先查找数据有没有改变 */
    res = memcmp(&(s_data_manager_pool[data_manager_dev->data_index]), data, data_manager_dev->data_size);
    if (res != 0) // 数据改变了，赋值数据
    {
        DATA_MANAGER_DEBUG_PRINTF("data change\n");
        osMutexWait(data_manager_dev->data_mutex,osWaitForever);//xSemaphoreTake(data_manager_dev->data_mutex,portMAX_DELAY);
        memcpy(&(s_data_manager_pool[data_manager_dev->data_index]), data, data_manager_dev->data_size);
        osMutexRelease(data_manager_dev->data_mutex);//xSemaphoreGive(data_manager_dev->data_mutex);
    }
    else
    {
        DATA_MANAGER_DEBUG_PRINTF("data not change\n");
    }
    return data_manager_dev->data_size;
}

/**
 * @note    获得数据
 * @param   data_manager_dev 数据对应的数据管理设备
 * @param   data 要获得的数据指针
 * @param   data_size 要获得的数据大小
 * @return  获得成功返回数据大小
 *          获得失败返回 0
 */
uint32_t get_data_manager_data(Data_manager_dev* data_manager_dev , void* data , uint32_t data_size)
{
    if (data_manager_dev == NULL)
        return 0;
    if (data_manager_dev->data_size != data_size)
        return 0;
    osMutexWait(data_manager_dev->data_mutex,osWaitForever);//xSemaphoreTake(data_manager_dev->data_mutex,portMAX_DELAY);
    memcpy(data, &(s_data_manager_pool[data_manager_dev->data_index]), data_manager_dev->data_size);
    osMutexRelease(data_manager_dev->data_mutex);//xSemaphoreGive(data_manager_dev->data_mutex);
    return data_manager_dev->data_size;
}

/**
 * @note    获得data manager内部信息
 * @param   Data_manager_info data manager内部信息结构体指针
 * @return  返回 0
 */
uint32_t get_data_manager_info(Data_manager_info* info)
{
    if (info == NULL)
        return 0;
    info->total_size = DATA_MANAGER_MAX_DATA_SIZE;
    info->used_size = s_data_manger_pool_index;
    info->left_size = DATA_MANAGER_MAX_DATA_SIZE - s_data_manger_pool_index;
    info->param_dev_total_size = DATA_MANAGER_MAX_DEV_NUM;
    info->param_dev_used_size = s_data_manger_index;
    info->param_dev_left_size = DATA_MANAGER_MAX_DEV_NUM - s_data_manger_index;
    return 0;
}
