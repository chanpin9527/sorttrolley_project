/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "system_tick.h"
#include "app_rs485.h"
#include "app_modbustcp_tx_rx.h"
#include "app_car_control.h"
#include "motor_driver.h"
#include "led_driver.h"
#include "raster_driver.h"
#include "app_eth_tcp_udp.h"
#include "app_diagnosis.h"
#include "easyflash.h"
#include "iwdg_driver.h"
#include "app_mqtt_tx.h"
#include "app_mqtt_rx.h"
#include "flash_parameter.h"
#include "usart_dma_driver.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define DEBUG_FREERTOS  // printf
#ifdef DEBUG_FREERTOS
#define print_freertos_log(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define print_freertos_log(fmt, ...)
#endif
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define PRINT_TICK_PERIOD	1000
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
void rt_show_version(void)
{
    printf("\n \\ | /\n");
    printf("- FreeRtos  Thread Operating System\n");
    printf(" / | \\     osCMSIS:%d.%02d,%s\n",
    		(osCMSIS>>16),(osCMSIS&0xffff),osKernelSystemId);
    printf("sort trolley systemboard software version: %d,haraware version: %d\r\n",RT_SOFTWARE_VERSION,RT_HARDWARE_VERSION);
    printf("sort trolley systemboard build %s\r\n",__DATE__);
}
/* USER CODE END Variables */
osThreadId defaultTaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
osThreadId carHandle;
osThreadId rs485Handle;
osThreadId ethHandle;
osThreadId diagnosisHandle;
osThreadId mqttTxHandle;
osThreadId mqttRXHandle;
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);

extern void MX_LWIP_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* GetTimerTaskMemory prototype (linked to static allocation support) */
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/* USER CODE BEGIN GET_TIMER_TASK_MEMORY */
static StaticTask_t xTimerTaskTCBBuffer;
static StackType_t xTimerStack[configTIMER_TASK_STACK_DEPTH];

void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize )
{
  *ppxTimerTaskTCBBuffer = &xTimerTaskTCBBuffer;
  *ppxTimerTaskStackBuffer = &xTimerStack[0];
  *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
  /* place for user code */
}
/* USER CODE END GET_TIMER_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
    EfErrCode res;
    rt_show_version();
	init_motor();
    creat_flash_manager_dev();
	res = easyflash_init();
    init_app_flash(res);
    rt_hw_usart_dma_init(_USART2_DEVICE_INDEX, NULL);
	iwdg_feeddog();
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
	init_set_data_manager();
	init_app485_data_manager();
	init_mqtt_rx_msg_mq();
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 512);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  
	osThreadDef(car_control, task_car_control, osPriorityNormal, 0, 512);
	carHandle = osThreadCreate(osThread(car_control), NULL);
	
	osThreadDef(rs485_rx_tx, task_rs485_tx, osPriorityAboveNormal, 0, 512);
	rs485Handle = osThreadCreate(osThread(rs485_rx_tx), NULL);
	
	osThreadDef(eth_task, task_eth_tcp_udp, osPriorityNormal, 0, 512);
	ethHandle = osThreadCreate(osThread(eth_task), NULL);
	
	osThreadDef(diagnosis_task, task_diagnosis, osPriorityNormal, 0, 256);
	diagnosisHandle = osThreadCreate(osThread(diagnosis_task), NULL);
	
	osThreadDef(mqttTx_task, task_mqtt_tx, osPriorityNormal, 0, 512);
	mqttTxHandle = osThreadCreate(osThread(mqttTx_task), NULL);
	
	osThreadDef(mqttRx_task, task_mqtt_rx, osPriorityNormal, 0, 256);
	mqttRXHandle = osThreadCreate(osThread(mqttRx_task), NULL);
  
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* init code for LWIP */
  MX_LWIP_Init();
  /* USER CODE BEGIN StartDefaultTask */
	uint64_t tick_pre = 0, tick_cur = 0, tick_led_pre = 0;
  /* Infinite loop */
  for(;;)
  {
	tick_cur = get_systick_time();
	if(tick_cur - tick_pre >= PRINT_TICK_PERIOD)
	{
		tick_pre = tick_cur;
		print_freertos_log("\r\n-----ticks:%d-----\r\n", (uint32_t )tick_cur);
	}
	tick_led_pre = set_led_control(tick_cur, tick_led_pre);
	osDelay(10);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
