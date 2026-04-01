#include "main.h"

#include "app.h"

#include "command_dispatcher.h"
#include "magnet.h"
#include "piston.h"
#include "rotator.h"
#include "step_generator.h"
#include "sys_config.h"
#include "sys_init.h"
#include "uart_receiver.h"

#define TEST_STEPPER (1)
#define TEST_PISTON ((0) && !TEST_STEPPER)

volatile uint32_t system_tick = 0;
volatile bool piston_moving = false;

void App_Run(void) {
  HAL_Delay(100);

#if TEST_STEPPER

  MoveBlock_t test_block_positive = StepGenerator_GenerateBlock(5000, 5000);
  MoveBlock_t test_block_negative = StepGenerator_GenerateBlock(-5000, -5000);
  for (;;) {
    bool throw = StepGenerator_StartMove(&test_block_positive);
    while (StepGenerator_IsBusy()) {
    }
    HAL_Delay(5000);

    throw = StepGenerator_StartMove(&test_block_negative);
    if (throw) {
    }
    while (StepGenerator_IsBusy()) {
    }
    HAL_Delay(5000);
    /*
        Magnet_SetState(true);
        HAL_Delay(500);
        Magnet_SetState(false);
        HAL_Delay(500); */
    /*
        if (Piston_Set(PISTON_POS_START) == PISTON_BUSY) {
        }

        HAL_Delay(500);
        if (Piston_Set(PISTON_POS_GRAB) == PISTON_BUSY) {
        }

        HAL_Delay(500);
        if (Piston_Set(PISTON_POS_START) == PISTON_BUSY) {
        }
        HAL_Delay(500);
        */
  }
#endif
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim) {
  if (htim->Instance == TIM2) {
    system_tick++;
    StepGenerator_Update();
    Rotator_Update();
    Piston_Update();
  }
}

/*
void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart) {
  if (huart->Instance == UART7) {
    UartReceiver_RxCallback(Sys_GetUartReceiver());
  }
}
*/