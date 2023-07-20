#ifndef __ADC_DRIVER_H_
#define __ADC_DRIVER_H_
#include "stdint.h"


enum
{
    _ADC1_DEVICE_INDEX = 0,
    _ADC2_DEVICE_INDEX = 1,
    _ADC3_DEVICE_INDEX = 2,
	_ADC3_DEVICE_MAX   = 3,
};


/**
 * @note      初始化adc dma
 * @param     *buf 缓存
 * @param     length 缓存长度
 * @return    返回结果
 */
int rt_hw_adc_dma_init(uint8_t adcx_n, uint16_t *buf, uint32_t length);

/*
 * @note    获取电压值
 * @return  执行结果
 */
float get_battery_voltage_value(uint8_t adcx_n, uint16_t *buf, uint32_t length);

#endif
