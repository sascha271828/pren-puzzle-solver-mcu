/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
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

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins
     PC14-OSC32_IN (OSC32_IN)   ------> RCC_OSC32_IN
     PC15-OSC32_OUT (OSC32_OUT)   ------> RCC_OSC32_OUT
     PH0-OSC_IN (PH0)   ------> RCC_OSC_IN
     PH1-OSC_OUT (PH1)   ------> RCC_OSC_OUT
     PD8   ------> USART3_TX
     PD9   ------> USART3_RX
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOF, DOUT_2_Pin|STEPPER_ROT_NSLEEP_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(STEPPER_ROT_STEP_GPIO_Port, STEPPER_ROT_STEP_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(STEPPER_X_NSLEEP_GPIO_Port, STEPPER_X_NSLEEP_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(DOUT_3_GPIO_Port, DOUT_3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, STEPPER_X_STEP_Pin|STEPPER_X_DIR_Pin|STEPPER_X_M1_Pin|DOUT_5_Pin
                          |DOUT_8_Pin|STEPPER_ROT_DIR_Pin|STEPPER_ROT_M1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, STEPPER_Y_ENABLE_Pin|STEPPER_Y_DIR_Pin|STEPPER_Y_M1_Pin|STEPPER_X_ENABLE_Pin
                          |STEPPER_X_M0_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, STEPPER_Y_M0_Pin|STEPPER_Y_STEP_Pin|DOUT_4_Pin|DOUT_6_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(STEPPER_Y_NSLEEP_GPIO_Port, STEPPER_Y_NSLEEP_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, STEPPER_ROT_M0_Pin|STEPPER_ROT_ENABLE_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(DOUT_7_GPIO_Port, DOUT_7_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(DOUT_1_GPIO_Port, DOUT_1_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : DIN_3_Pin DIN_1_Pin STEPPER_X_NFAULT_Pin STEPPER_Y_NFAULT_Pin
                           DIN_6_Pin */
  GPIO_InitStruct.Pin = DIN_3_Pin|DIN_1_Pin|STEPPER_X_NFAULT_Pin|STEPPER_Y_NFAULT_Pin
                          |DIN_6_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : DIN_12_Pin DIN_10_Pin DIN_8_Pin DIN_7_Pin
                           DIN_5_Pin DIN_9_Pin DIN_2_Pin */
  GPIO_InitStruct.Pin = DIN_12_Pin|DIN_10_Pin|DIN_8_Pin|DIN_7_Pin
                          |DIN_5_Pin|DIN_9_Pin|DIN_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : DOUT_2_Pin STEPPER_ROT_NSLEEP_Pin */
  GPIO_InitStruct.Pin = DOUT_2_Pin|STEPPER_ROT_NSLEEP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pin : STEPPER_ROT_STEP_Pin */
  GPIO_InitStruct.Pin = STEPPER_ROT_STEP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(STEPPER_ROT_STEP_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : STEPPER_X_NSLEEP_Pin DOUT_3_Pin */
  GPIO_InitStruct.Pin = STEPPER_X_NSLEEP_Pin|DOUT_3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : STEPPER_X_STEP_Pin STEPPER_X_DIR_Pin STEPPER_X_M1_Pin STEPPER_ROT_DIR_Pin
                           STEPPER_ROT_M1_Pin */
  GPIO_InitStruct.Pin = STEPPER_X_STEP_Pin|STEPPER_X_DIR_Pin|STEPPER_X_M1_Pin|STEPPER_ROT_DIR_Pin
                          |STEPPER_ROT_M1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : DIN_4_Pin */
  GPIO_InitStruct.Pin = DIN_4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(DIN_4_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : STEPPER_Y_ENABLE_Pin STEPPER_X_ENABLE_Pin */
  GPIO_InitStruct.Pin = STEPPER_Y_ENABLE_Pin|STEPPER_X_ENABLE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : STEPPER_Y_DIR_Pin STEPPER_Y_M1_Pin STEPPER_X_M0_Pin */
  GPIO_InitStruct.Pin = STEPPER_Y_DIR_Pin|STEPPER_Y_M1_Pin|STEPPER_X_M0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : STLINK_RX_Pin STLINK_TX_Pin */
  GPIO_InitStruct.Pin = STLINK_RX_Pin|STLINK_TX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : STEPPER_Y_M0_Pin STEPPER_Y_STEP_Pin */
  GPIO_InitStruct.Pin = STEPPER_Y_M0_Pin|STEPPER_Y_STEP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : STEPPER_Y_NSLEEP_Pin DOUT_4_Pin DOUT_6_Pin */
  GPIO_InitStruct.Pin = STEPPER_Y_NSLEEP_Pin|DOUT_4_Pin|DOUT_6_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : STEPPER_ROT_M0_Pin */
  GPIO_InitStruct.Pin = STEPPER_ROT_M0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(STEPPER_ROT_M0_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : DOUT_7_Pin */
  GPIO_InitStruct.Pin = DOUT_7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(DOUT_7_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : STEPPER_ROT_NFAULT_Pin */
  GPIO_InitStruct.Pin = STEPPER_ROT_NFAULT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(STEPPER_ROT_NFAULT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : STEPPER_ROT_ENABLE_Pin */
  GPIO_InitStruct.Pin = STEPPER_ROT_ENABLE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(STEPPER_ROT_ENABLE_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : DOUT_5_Pin DOUT_1_Pin DOUT_8_Pin */
  GPIO_InitStruct.Pin = DOUT_5_Pin|DOUT_1_Pin|DOUT_8_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*AnalogSwitch Config */
  HAL_SYSCFG_AnalogSwitchConfig(SYSCFG_SWITCH_PA0, SYSCFG_SWITCH_PA0_CLOSE);

}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */
