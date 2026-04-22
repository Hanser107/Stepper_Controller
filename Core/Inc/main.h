/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "stm32f1xx_hal.h"

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
#define Motor2_Dir_Pin GPIO_PIN_1
#define Motor2_Dir_GPIO_Port GPIOA
#define Motor2_Ena_Pin GPIO_PIN_2
#define Motor2_Ena_GPIO_Port GPIOA
#define Extend_IO1_Pin GPIO_PIN_3
#define Extend_IO1_GPIO_Port GPIOA
#define Extend_IO2_Pin GPIO_PIN_4
#define Extend_IO2_GPIO_Port GPIOA
#define Extend_IO3_Pin GPIO_PIN_5
#define Extend_IO3_GPIO_Port GPIOA
#define Motor3_DIr_Pin GPIO_PIN_0
#define Motor3_DIr_GPIO_Port GPIOB
#define Motor3_Ena_Pin GPIO_PIN_1
#define Motor3_Ena_GPIO_Port GPIOB
#define Motor1_Ena_Pin GPIO_PIN_14
#define Motor1_Ena_GPIO_Port GPIOB
#define Motor1_Dir_Pin GPIO_PIN_15
#define Motor1_Dir_GPIO_Port GPIOB
#define Motor4_Ena_Pin GPIO_PIN_4
#define Motor4_Ena_GPIO_Port GPIOB
#define Motor4_Dir_Pin GPIO_PIN_5
#define Motor4_Dir_GPIO_Port GPIOB
#define KEY1_Pin GPIO_PIN_7
#define KEY1_GPIO_Port GPIOB
#define LED1_Pin GPIO_PIN_8
#define LED1_GPIO_Port GPIOB
#define LED2_Pin GPIO_PIN_9
#define LED2_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
