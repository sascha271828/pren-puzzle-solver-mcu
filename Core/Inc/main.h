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
#include "stm32h7xx_hal.h"

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
void Start_Piston_Timer(uint32_t delay_ms);
extern volatile uint32_t system_tick;

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define STEPPER_Y_NSLEEP_Pin GPIO_PIN_2
#define STEPPER_Y_NSLEEP_GPIO_Port GPIOE
#define STEPPER_Y_ENABLE_Pin GPIO_PIN_3
#define STEPPER_Y_ENABLE_GPIO_Port GPIOE
#define STEPPER_ROT_NSLEEP_Pin GPIO_PIN_4
#define STEPPER_ROT_NSLEEP_GPIO_Port GPIOE
#define STEPPER_ROT_ENABLE_Pin GPIO_PIN_5
#define STEPPER_ROT_ENABLE_GPIO_Port GPIOE
#define DOUT_1_Pin GPIO_PIN_0
#define DOUT_1_GPIO_Port GPIOF
#define DOUT_2_Pin GPIO_PIN_1
#define DOUT_2_GPIO_Port GPIOF
#define DOUT_3_Pin GPIO_PIN_2
#define DOUT_3_GPIO_Port GPIOF
#define DOUT_4_Pin GPIO_PIN_3
#define DOUT_4_GPIO_Port GPIOF
#define DOUT_5_Pin GPIO_PIN_4
#define DOUT_5_GPIO_Port GPIOF
#define DOUT_6_Pin GPIO_PIN_5
#define DOUT_6_GPIO_Port GPIOF
#define UART_RX_Pin GPIO_PIN_6
#define UART_RX_GPIO_Port GPIOF
#define UART_TX_Pin GPIO_PIN_7
#define UART_TX_GPIO_Port GPIOF
#define DOUT_7_Pin GPIO_PIN_9
#define DOUT_7_GPIO_Port GPIOF
#define DOUT_8_Pin GPIO_PIN_10
#define DOUT_8_GPIO_Port GPIOF
#define DIN_1_Pin GPIO_PIN_1
#define DIN_1_GPIO_Port GPIOB
#define DIN_2_Pin GPIO_PIN_2
#define DIN_2_GPIO_Port GPIOB
#define STEPPER_X_NFAULT_Pin GPIO_PIN_7
#define STEPPER_X_NFAULT_GPIO_Port GPIOE
#define STEPPER_Y_NFAULT_Pin GPIO_PIN_8
#define STEPPER_Y_NFAULT_GPIO_Port GPIOE
#define STEPPER_ROT_NFAULT_Pin GPIO_PIN_9
#define STEPPER_ROT_NFAULT_GPIO_Port GPIOE
#define DIN_9_Pin GPIO_PIN_10
#define DIN_9_GPIO_Port GPIOB
#define DIN_10_Pin GPIO_PIN_11
#define DIN_10_GPIO_Port GPIOB
#define DIN_11_Pin GPIO_PIN_12
#define DIN_11_GPIO_Port GPIOB
#define DIN_12_Pin GPIO_PIN_13
#define DIN_12_GPIO_Port GPIOB
#define STLINK_RX_Pin GPIO_PIN_8
#define STLINK_RX_GPIO_Port GPIOD
#define STLINK_TX_Pin GPIO_PIN_9
#define STLINK_TX_GPIO_Port GPIOD
#define STEPPER_X_M0_Pin GPIO_PIN_10
#define STEPPER_X_M0_GPIO_Port GPIOD
#define STEPPER_X_M1_Pin GPIO_PIN_11
#define STEPPER_X_M1_GPIO_Port GPIOD
#define STEPPER_Y_M0_Pin GPIO_PIN_12
#define STEPPER_Y_M0_GPIO_Port GPIOD
#define STEPPER_Y_M1_Pin GPIO_PIN_13
#define STEPPER_Y_M1_GPIO_Port GPIOD
#define STEPPER_ROT_M0_Pin GPIO_PIN_14
#define STEPPER_ROT_M0_GPIO_Port GPIOD
#define STEPPER_ROT_M1_Pin GPIO_PIN_15
#define STEPPER_ROT_M1_GPIO_Port GPIOD
#define STEPPER_X_STEP_Pin GPIO_PIN_0
#define STEPPER_X_STEP_GPIO_Port GPIOD
#define STEPPER_Y_STEP_Pin GPIO_PIN_1
#define STEPPER_Y_STEP_GPIO_Port GPIOD
#define STEPPER_ROT_STEP_Pin GPIO_PIN_3
#define STEPPER_ROT_STEP_GPIO_Port GPIOD
#define STEPPER_X_DIR_Pin GPIO_PIN_4
#define STEPPER_X_DIR_GPIO_Port GPIOD
#define STEPPER_Y_DIR_Pin GPIO_PIN_5
#define STEPPER_Y_DIR_GPIO_Port GPIOD
#define STEPPER_ROT_DIR_Pin GPIO_PIN_7
#define STEPPER_ROT_DIR_GPIO_Port GPIOD
#define DIN_3_Pin GPIO_PIN_3
#define DIN_3_GPIO_Port GPIOB
#define DIN_4_Pin GPIO_PIN_4
#define DIN_4_GPIO_Port GPIOB
#define DIN_5_Pin GPIO_PIN_5
#define DIN_5_GPIO_Port GPIOB
#define DIN_6_Pin GPIO_PIN_6
#define DIN_6_GPIO_Port GPIOB
#define DIN_7_Pin GPIO_PIN_8
#define DIN_7_GPIO_Port GPIOB
#define DIN_8_Pin GPIO_PIN_9
#define DIN_8_GPIO_Port GPIOB
#define STEPPER_X_NSLEEP_Pin GPIO_PIN_0
#define STEPPER_X_NSLEEP_GPIO_Port GPIOE
#define STEPPER_X_ENABLE_Pin GPIO_PIN_1
#define STEPPER_X_ENABLE_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
