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
 * @note      ��ʼ��adc dma
 * @param     *buf ����
 * @param     length ���泤��
 * @return    ���ؽ��
 */
int rt_hw_adc_dma_init(uint8_t adcx_n, uint16_t *buf, uint32_t length);

/*
 * @note    ��ȡ��ѹֵ
 * @return  ִ�н��
 */
float get_battery_voltage_value(uint8_t adcx_n, uint16_t *buf, uint32_t length);

#endif
