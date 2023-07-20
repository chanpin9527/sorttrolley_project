#ifndef __RASTER_DRIVER_H_
#define __RASTER_DRIVER_H_

#include "main.h"


#define COUNTER_RASTER_STATE	HAL_GPIO_ReadPin(COUNTER_RASTER_GPIO_Port, COUNTER_RASTER_Pin)
#define RESET_RASTER_STATE		HAL_GPIO_ReadPin(RESET_RASTER_GPIO_Port, RESET_RASTER_Pin)

typedef void (*raster_irq_handler)(void);

uint8_t	read_counter_raster_statue(void);

uint8_t	read_reset_raster_statue(void);

void register_raster_irq_haaandle(raster_irq_handler counter_handler, raster_irq_handler reset_handler);

#endif
