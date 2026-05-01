#include "main.h"

#include "app.h"

#include "buttons.h"
#include "command_dispatcher.h"
#include "emergency_stop.h"
#include "homer.h"
#include "interrupt.h"
#include "limit_switch.h"
#include "magnet.h"
#include "piston.h"
#include "rotator.h"
#include "state_machine.h"
#include "step_generator.h"
#include "sys_config.h"
#include "sys_init.h"
#include "uart_receiver.h"
#include "usart.h"

#if RUN_MODE == RUN_MODE_TEST_CLI
#include "test_cli.h"
#endif
#define TEST_ISR_TIME 0

/* ── Entry point ───────────────────────────────────────────────────────────
 */
void App_Run(void) {
  HAL_Delay(500); /* let peripherals settle */

#if RUN_MODE == RUN_MODE_TEST_CLI

  /* LED test*/
  //  while (true) {
  //    if (HAL_GPIO_ReadPin(DIN_1_GPIO_Port, DIN_1_Pin) == GPIO_PIN_SET) {
  //      HAL_GPIO_WritePin(DOUT_2_GPIO_Port, DOUT_2_Pin, GPIO_PIN_RESET);
  //    } else {
  //      HAL_GPIO_WritePin(DOUT_2_GPIO_Port, DOUT_2_Pin, GPIO_PIN_SET);
  //    }
  //    if (HAL_GPIO_ReadPin(DIN_2_GPIO_Port, DIN_2_Pin) == GPIO_PIN_SET) {
  //      HAL_GPIO_WritePin(DOUT_2_GPIO_Port, DOUT_2_Pin, GPIO_PIN_RESET);
  //    } else {
  //      HAL_GPIO_WritePin(DOUT_2_GPIO_Port, DOUT_2_Pin, GPIO_PIN_SET);
  //    }
  //    if (HAL_GPIO_ReadPin(DIN_3_GPIO_Port, DIN_3_Pin) == GPIO_PIN_SET) {
  //      HAL_GPIO_WritePin(DOUT_2_GPIO_Port, DOUT_2_Pin, GPIO_PIN_RESET);
  //    } else {
  //      HAL_GPIO_WritePin(DOUT_2_GPIO_Port, DOUT_2_Pin, GPIO_PIN_SET);
  //    }
  //    if (HAL_GPIO_ReadPin(DIN_4_GPIO_Port, DIN_4_Pin) == GPIO_PIN_SET) {
  //      HAL_GPIO_WritePin(DOUT_2_GPIO_Port, DOUT_2_Pin, GPIO_PIN_RESET);
  //    } else {
  //      HAL_GPIO_WritePin(DOUT_2_GPIO_Port, DOUT_2_Pin, GPIO_PIN_SET);
  //    }
  //  }

  /* STEPPER Y */
  //  HAL_GPIO_WritePin(
  //      STEPPER_Y_ENABLE_GPIO_Port, STEPPER_Y_ENABLE_Pin, GPIO_PIN_RESET);
  //
  //  HAL_GPIO_WritePin(STEPPER_Y_DIR_GPIO_Port, STEPPER_Y_DIR_Pin,
  //  GPIO_PIN_RESET);
  //
  //  HAL_GPIO_WritePin(STEPPER_Y_M0_GPIO_Port, STEPPER_Y_M0_Pin,
  //  GPIO_PIN_RESET);
  //
  //  HAL_GPIO_WritePin(STEPPER_Y_M1_GPIO_Port, STEPPER_Y_M1_Pin,
  //  GPIO_PIN_RESET);
  //
  //  HAL_GPIO_WritePin(
  //      STEPPER_Y_NSLEEP_GPIO_Port, STEPPER_Y_NSLEEP_Pin, GPIO_PIN_RESET);
  //
  //  while (true) {
  //    HAL_GPIO_WritePin(
  //        STEPPER_Y_STEP_GPIO_Port, STEPPER_Y_STEP_Pin, GPIO_PIN_SET);
  //
  //    HAL_GPIO_WritePin(
  //        STEPPER_Y_STEP_GPIO_Port, STEPPER_Y_STEP_Pin, GPIO_PIN_RESET);
  //
  //    HAL_GPIO_WritePin(
  //        STEPPER_Y_ENABLE_GPIO_Port, STEPPER_Y_ENABLE_Pin, GPIO_PIN_SET);
  //    HAL_GPIO_WritePin(
  //        STEPPER_Y_ENABLE_GPIO_Port, STEPPER_Y_ENABLE_Pin, GPIO_PIN_RESET);
  //
  //    HAL_GPIO_WritePin(STEPPER_Y_DIR_GPIO_Port, STEPPER_Y_DIR_Pin,
  //    GPIO_PIN_SET); HAL_GPIO_WritePin(
  //        STEPPER_Y_DIR_GPIO_Port, STEPPER_Y_DIR_Pin, GPIO_PIN_RESET);
  //
  //    HAL_GPIO_WritePin(STEPPER_Y_M0_GPIO_Port, STEPPER_Y_M0_Pin,
  //    GPIO_PIN_SET); HAL_GPIO_WritePin(STEPPER_Y_M0_GPIO_Port,
  //    STEPPER_Y_M0_Pin, GPIO_PIN_RESET);
  //
  //    HAL_GPIO_WritePin(STEPPER_Y_M1_GPIO_Port, STEPPER_Y_M1_Pin,
  //    GPIO_PIN_SET); HAL_GPIO_WritePin(STEPPER_Y_M1_GPIO_Port,
  //    STEPPER_Y_M1_Pin, GPIO_PIN_RESET);
  //
  //    HAL_GPIO_WritePin(
  //        STEPPER_Y_NSLEEP_GPIO_Port, STEPPER_Y_NSLEEP_Pin, GPIO_PIN_SET);
  //    HAL_GPIO_WritePin(
  //        STEPPER_Y_NSLEEP_GPIO_Port, STEPPER_Y_NSLEEP_Pin, GPIO_PIN_RESET);
  //  }

  TestCLI_Init(&huart3);
  TestCLI_Run(); /* never returns */

#elif RUN_MODE == RUN_MODE_APP
  /* --- STATE MACHINE MODUS --- */
  CommandDispatcher_t* dispatcher = Sys_GetCommandDispatcher();

  StateMachine_Init(dispatcher);

  Piston_Set(PISTON_POS_START);
  Magnet_SetState(false);

  for (;;) {
    CommandDispatcher_Poll(dispatcher);
    StateMachine_Update();
  }

#else
#error "RUN_MODE not defined or unknown — set it in sys_config.h"
#endif
}

/* ── ISR ───────────────────────────────────────────────────────────────────
 */

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim) {
  if (htim->Instance == TIM2) {
#if TEST_ISR_TIME
    HAL_GPIO_WritePin(DOUT_8_GPIO_Port, DOUT_8_Pin, GPIO_PIN_SET);
#endif
    EmergencyStop_Process();
    Buttons_Poll_ISR();

    switch (Interrupt_GetState()) {
      case IS_HOMING:
        Homer_Update();
        break;
      case IS_READY:
      case IS_RUNNING:
        if (LimitSwitch_Activated()) {
          StepGenerator_Abort();
        } else {
          StepGenerator_Update();
        }
        Rotator_Update();
        Piston_Update();
        break;
      case IS_ESTOP:
        Magnet_SetState(false);
        StepGenerator_Abort();
        Rotator_Abort();
        Piston_Abort();
        if (EmergencyStop_IsActivated() == false) {
          Buttons_Reset_RearmPressDetection();
          if (Buttons_Reset_Pressed() == true) {
            Homer_HomingStart();
          }
        }
        break;
      case IS_INIT:
        Piston_Update();
      default:
        break;
    }
#if TEST_ISR_TIME
    HAL_GPIO_WritePin(DOUT_8_GPIO_Port, DOUT_8_Pin, GPIO_PIN_RESET);
#endif
  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart) {
  if (huart->Instance == UART7) {
    UartReceiver_RxCallback(Sys_GetUartReceiver());
  }
}