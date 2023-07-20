/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define ETH_MDC_Pin GPIO_PIN_1
#define ETH_MDC_GPIO_Port GPIOC
#define ETH_MDIO_Pin GPIO_PIN_2
#define ETH_MDIO_GPIO_Port GPIOA
#define ETH_CRS_DV_Pin GPIO_PIN_7
#define ETH_CRS_DV_GPIO_Port GPIOA
#define ETH_RXD0_Pin GPIO_PIN_4
#define ETH_RXD0_GPIO_Port GPIOC
#define ETH_RXD1_Pin GPIO_PIN_5
#define ETH_RXD1_GPIO_Port GPIOC
#define RAIL_2_Pin GPIO_PIN_10
#define RAIL_2_GPIO_Port GPIOE
#define RAIL_1_Pin GPIO_PIN_11
#define RAIL_1_GPIO_Port GPIOE
#define RAIL_0_Pin GPIO_PIN_12
#define RAIL_0_GPIO_Port GPIOE
#define CARID2_Pin GPIO_PIN_13
#define CARID2_GPIO_Port GPIOE
#define CARID1_Pin GPIO_PIN_14
#define CARID1_GPIO_Port GPIOE
#define CARID0_Pin GPIO_PIN_15
#define CARID0_GPIO_Port GPIOE
#define LED_Pin GPIO_PIN_10
#define LED_GPIO_Port GPIOB
#define ETH_TX_EN_Pin GPIO_PIN_11
#define ETH_TX_EN_GPIO_Port GPIOB
#define ETH_TXD0_Pin GPIO_PIN_12
#define ETH_TXD0_GPIO_Port GPIOB
#define ETH_TXD1_Pin GPIO_PIN_13
#define ETH_TXD1_GPIO_Port GPIOB
#define COUNTER_RASTER_Pin GPIO_PIN_14
#define COUNTER_RASTER_GPIO_Port GPIOB
#define RESET_RASTER_Pin GPIO_PIN_15
#define RESET_RASTER_GPIO_Port GPIOB
#define LIMIT1_Pin GPIO_PIN_10
#define LIMIT1_GPIO_Port GPIOD
#define LIMIT2_Pin GPIO_PIN_11
#define LIMIT2_GPIO_Port GPIOD
#define LIMIT3_Pin GPIO_PIN_12
#define LIMIT3_GPIO_Port GPIOD
#define LIMIT4_Pin GPIO_PIN_13
#define LIMIT4_GPIO_Port GPIOD
#define LIMIT5_Pin GPIO_PIN_14
#define LIMIT5_GPIO_Port GPIOD
#define LIMIT6_Pin GPIO_PIN_15
#define LIMIT6_GPIO_Port GPIOD
#define LIMIT7_Pin GPIO_PIN_6
#define LIMIT7_GPIO_Port GPIOC
#define LIMIT8_Pin GPIO_PIN_7
#define LIMIT8_GPIO_Port GPIOC
#define LIMIT9_Pin GPIO_PIN_8
#define LIMIT9_GPIO_Port GPIOC
#define LIMIT10_Pin GPIO_PIN_9
#define LIMIT10_GPIO_Port GPIOC
#define LIMIT11_Pin GPIO_PIN_8
#define LIMIT11_GPIO_Port GPIOA
#define LIMIT12_Pin GPIO_PIN_11
#define LIMIT12_GPIO_Port GPIOA
#define LIMIT13_Pin GPIO_PIN_12
#define LIMIT13_GPIO_Port GPIOA
#define SPI1_CS_Pin GPIO_PIN_15
#define SPI1_CS_GPIO_Port GPIOA
#define LIMIT14_Pin GPIO_PIN_10
#define LIMIT14_GPIO_Port GPIOC
#define LIMIT15_Pin GPIO_PIN_11
#define LIMIT15_GPIO_Port GPIOC
#define MOTOR_BRAKE_Pin GPIO_PIN_2
#define MOTOR_BRAKE_GPIO_Port GPIOD
#define MOTOR_DIRECTION_Pin GPIO_PIN_3
#define MOTOR_DIRECTION_GPIO_Port GPIOD
#define RS485_TX_Pin GPIO_PIN_5
#define RS485_TX_GPIO_Port GPIOD
#define RS485_RX_Pin GPIO_PIN_6
#define RS485_RX_GPIO_Port GPIOD
#define RS485_DE_Pin GPIO_PIN_7
#define RS485_DE_GPIO_Port GPIOD
#define RUN_LED_Pin GPIO_PIN_6
#define RUN_LED_GPIO_Port GPIOB
#define FAULT_LED_Pin GPIO_PIN_7
#define FAULT_LED_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */
#define PRINTF_DEBUG_ON	0


#define PRODUCT_TYPE                "1.0.0"

#define SOFTWARE_MAIN_VERSION	   (1)
#define SOFTWARE_SUB_VERSION	   (0)
#define SOFTWARE_REVISION          (0)

#define HARDWARE_LINE_VERSION	   (100)
#define HARDWARE_PRODUCT_VERSION   (1)
#define HARDWARE_BORAD_REVERSION   (1)
#define HARDWARE_REVISION	       (1)

#define RT_SOFTWARE_VERSION        (1000000*SOFTWARE_MAIN_VERSION + 1000*SOFTWARE_SUB_VERSION + SOFTWARE_REVISION)
#define RT_HARDWARE_VERSION        (10000000*HARDWARE_LINE_VERSION + 10000*HARDWARE_PRODUCT_VERSION+100*HARDWARE_BORAD_REVERSION + HARDWARE_REVISION)
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
