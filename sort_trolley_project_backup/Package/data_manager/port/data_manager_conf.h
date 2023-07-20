/*
 * description：用于配置data manager的文件
 * version: 1.0
 * Change Logs:
 * Date           Author           Notes
 * 2019-03-06     zhaojinlong      create
 * 2020-06-10     zhaojinlong      增加debug配置
*/

#ifndef __DATA_MANAGER_CONF_H__
#define __DATA_MANAGER_CONF_H__

/* 是否开启debug */
//#define DEBUG_DATA_MANAGER

/* data manager内部索引的名字最大的长度，名字越长占用的内存会越多 */
#define DATA_MANAGER_NAME_MAX          20

/* data manager有多少个数据结构需要管理，越长占用的内存会越多 */
#define DATA_MANAGER_MAX_DEV_NUM       25

/* data manager内部缓存用来存放数据，越长占用的内存会越多 */
#define DATA_MANAGER_MAX_DATA_SIZE    4096
#endif

