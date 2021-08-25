/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2019 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define __RXLEN 128
#define __TXLEN 128
#define __CMDLEN 128
#define __PWMRATE 2000
#define __VALVES 4
#define __LEDS 80
#define __TH 94
#define __TL 36
#define __TP 130
#define SW_version 108
#define FSW2_Pin GPIO_PIN_2
#define FSW2_GPIO_Port GPIOE
#define FSW3_Pin GPIO_PIN_3
#define FSW3_GPIO_Port GPIOE
#define PE4_Pin GPIO_PIN_4
#define PE4_GPIO_Port GPIOE
#define Psense0_Pin GPIO_PIN_0
#define Psense0_GPIO_Port GPIOC
#define Psense1_Pin GPIO_PIN_1
#define Psense1_GPIO_Port GPIOC
#define Psense2_Pin GPIO_PIN_2
#define Psense2_GPIO_Port GPIOC
#define Psense3_Pin GPIO_PIN_3
#define Psense3_GPIO_Port GPIOC
#define PUMP_CSENSE_Pin GPIO_PIN_0
#define PUMP_CSENSE_GPIO_Port GPIOA
#define T1_H2O_Pin GPIO_PIN_1
#define T1_H2O_GPIO_Port GPIOA
#define T1_H2OA2_Pin GPIO_PIN_2
#define T1_H2OA2_GPIO_Port GPIOA
#define _5V_Pin GPIO_PIN_3
#define _5V_GPIO_Port GPIOA
#define PUMP_TACHO_Pin GPIO_PIN_6
#define PUMP_TACHO_GPIO_Port GPIOA
#define FLOW_TACHO_Pin GPIO_PIN_7
#define FLOW_TACHO_GPIO_Port GPIOA
#define DLsense1_Pin GPIO_PIN_4
#define DLsense1_GPIO_Port GPIOC
#define DLsense2_Pin GPIO_PIN_5
#define DLsense2_GPIO_Port GPIOC
#define _12V_Pin GPIO_PIN_0
#define _12V_GPIO_Port GPIOB
#define _24V_Pin GPIO_PIN_1
#define _24V_GPIO_Port GPIOB
#define PE7_Pin GPIO_PIN_7
#define PE7_GPIO_Port GPIOE
#define PE8_Pin GPIO_PIN_8
#define PE8_GPIO_Port GPIOE
#define PE10_Pin GPIO_PIN_10
#define PE10_GPIO_Port GPIOE
#define PE12_Pin GPIO_PIN_12
#define PE12_GPIO_Port GPIOE
#define PE15_Pin GPIO_PIN_15
#define PE15_GPIO_Port GPIOE
#define DSenseTIM1_Pin GPIO_PIN_14
#define DSenseTIM1_GPIO_Port GPIOB
#define DSenseTIM1B15_Pin GPIO_PIN_15
#define DSenseTIM1B15_GPIO_Port GPIOB
#define cwbBUTTON_Pin GPIO_PIN_9
#define cwbBUTTON_GPIO_Port GPIOD
#define cwbDOOR_Pin GPIO_PIN_10
#define cwbDOOR_GPIO_Port GPIOD
#define cwbENGM_Pin GPIO_PIN_11
#define cwbENGM_GPIO_Port GPIOD
#define _LED1_Pin GPIO_PIN_12
#define _LED1_GPIO_Port GPIOD
#define _LED2_Pin GPIO_PIN_13
#define _LED2_GPIO_Port GPIOD
#define FSW_DL_Pin GPIO_PIN_14
#define FSW_DL_GPIO_Port GPIOD
#define FSW_EC_Pin GPIO_PIN_15
#define FSW_EC_GPIO_Port GPIOD
#define _BOTTLE_OUT_Pin GPIO_PIN_6
#define _BOTTLE_OUT_GPIO_Port GPIOC
#define _BOTTLE_IN_Pin GPIO_PIN_7
#define _BOTTLE_IN_GPIO_Port GPIOC
#define _AIR_Pin GPIO_PIN_8
#define _AIR_GPIO_Port GPIOC
#define _WATER_Pin GPIO_PIN_9
#define _WATER_GPIO_Port GPIOC
#define cwbOVRD_Pin GPIO_PIN_8
#define cwbOVRD_GPIO_Port GPIOA
#define _RED1_Pin GPIO_PIN_0
#define _RED1_GPIO_Port GPIOD
#define _GREEN1_Pin GPIO_PIN_1
#define _GREEN1_GPIO_Port GPIOD
#define _YELLOW1_Pin GPIO_PIN_2
#define _YELLOW1_GPIO_Port GPIOD
#define _BLUE1_Pin GPIO_PIN_3
#define _BLUE1_GPIO_Port GPIOD
#define _RED2_Pin GPIO_PIN_4
#define _RED2_GPIO_Port GPIOD
#define _GREEN2_Pin GPIO_PIN_5
#define _GREEN2_GPIO_Port GPIOD
#define _YELLOW2_Pin GPIO_PIN_6
#define _YELLOW2_GPIO_Port GPIOD
#define _BLUE2_Pin GPIO_PIN_7
#define _BLUE2_GPIO_Port GPIOD
#define _12Voff_Pin GPIO_PIN_3
#define _12Voff_GPIO_Port GPIOB
#define _SYS_SHG_Pin GPIO_PIN_4
#define _SYS_SHG_GPIO_Port GPIOB
#define FAN_PWM_Pin GPIO_PIN_8
#define FAN_PWM_GPIO_Port GPIOB
#define FSW0_Pin GPIO_PIN_0
#define FSW0_GPIO_Port GPIOE
#define FSW1_Pin GPIO_PIN_1
#define FSW1_GPIO_Port GPIOE
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
