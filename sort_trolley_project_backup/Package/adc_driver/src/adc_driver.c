#include "data_manager.h"
#include "adc_driver.h"
#include "adc.h"

#pragma pack(1)

#define DEBUG_ADC_DRIVER  // printf
#ifdef DEBUG_ADC_DRIVER
#define print_adc_log(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define print_adc_log(fmt, ...)
#endif


#define AVERAGE_BUFFER_SIZE 2
static uint16_t s_adc_average_buffer[_ADC3_DEVICE_MAX][AVERAGE_BUFFER_SIZE] = {0};
typedef struct
{
	ADC_HandleTypeDef* adcx;
	void* buffer;
	uint16_t readCnt;
	uint16_t writeCnt;
	uint16_t size;
}Average_typedef;

static Average_typedef s_adc_struct[] = {
	[_ADC2_DEVICE_INDEX] = {&hadc2, (void *)s_adc_average_buffer[_ADC2_DEVICE_INDEX],0,0,AVERAGE_BUFFER_SIZE},
	[_ADC3_DEVICE_INDEX] = {&hadc3, (void *)s_adc_average_buffer[_ADC3_DEVICE_INDEX],0,0,AVERAGE_BUFFER_SIZE}
};

/**
 * @note      初始化adc dma
 * @param     *buf 缓存
 * @param     length 缓存长度
 * @return    返回结果
 */
int rt_hw_adc_dma_init(uint8_t adcx_n, uint16_t *buf,uint32_t length)
{
	if(adcx_n < _ADC1_DEVICE_INDEX || adcx_n > _ADC3_DEVICE_INDEX)
		return -1;
	
	if(HAL_ADC_Start_DMA(s_adc_struct[adcx_n].adcx,(uint32_t *)buf,length) != HAL_OK)
	{
		Error_Handler();
		return -1;
	}
	return 0;
}

/*
 * @note    平均值计算函数
 * @param   *check_temp_concentration 检测值
 * @param   *check_temp_parameter 运行参数
 * @return  执行结果
 */
static uint16_t averagefifo_function(uint16_t import,Average_typedef *average)
{
	uint16_t *buffer = (uint16_t *)average->buffer ;
	uint32_t aver_sum = 0;
	uint8_t j=0;
	buffer[(average->writeCnt++)] = import;
	if(average->writeCnt >= average->size)
	{
		average->writeCnt = 0;
	}
	for(j = 0;j<average->size ;j++)
	{
		aver_sum +=buffer[j];
	}
	return aver_sum/average->size;
}

/*
 * @note    获取电压值
 * @return  执行结果
 */
float get_battery_voltage_value(uint8_t adcx_n, uint16_t *buf,uint32_t length)
{
	float battery_voltage = 0;
	uint32_t i = 0;
	uint32_t average = 0;
	uint32_t sum = 0;
	const float vol = 2/33.0;
	
	if(adcx_n < _ADC1_DEVICE_INDEX || adcx_n > _ADC3_DEVICE_INDEX)
		return -1;
	for(i=0;i<length;i++)
	{
		sum += buf[i];
	}
	average = (uint32_t)sum/length;
	average = averagefifo_function(average, &s_adc_struct[adcx_n]);
//	battery_voltage = (float)(3300 * average)/4096;
//	battery_voltage = battery_voltage*4.24/1000 + 0.65;
	battery_voltage = (float)(average * 3.3)/4096;
	battery_voltage = battery_voltage / vol;
    return battery_voltage;
}

#pragma pack()

