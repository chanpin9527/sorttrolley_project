/*
 * description：flash_parameter用于所有参数的读写
 * version: 1.0
 * Change Logs:
 * Date           Author           Notes
 * 2023-04-15     litao            create                       
 */

#ifndef __FLASH_PARAMETER_H__
#define __FLASH_PARAMETER_H__
#include <stdint.h>

#pragma pack(1) //内存字节对齐

/*
 * @note    创建所有参数管理描述符
 * @param   None
 * @return   
 */
int32_t creat_flash_manager_dev(void);

/*
 * @note    初始化应用参数
 * @param   status spiflash是否ok
 * @return   
 */
int32_t init_app_flash(int32_t flash_status);

/*
 * @note    设置flash数据里的包裹信息
 * @param   *env_para 包裹参数
 * @param   number 滚筒号
 * @return   
 */
int32_t set_package_info_flash_param(const char *env_para,uint8_t number);

/*
 * @note    设置flash数据里的配置参数
 * @param   None
 * @return   
 */
int32_t set_operating_para_flash_param(const char *env_para);


/*
 * @note    设置flash单个运行数据
 * @param   None
 * @return   
 */
int32_t set_single_data_flash_param(const char *key, const void *value_buf, size_t buf_len);

#pragma pack()
#endif
