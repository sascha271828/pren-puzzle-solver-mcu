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

#if RUN_MODE == RUN_MODE_TEST_STATE
static PuzzleCommand test_cmd = PuzzleCommand_init_zero;
#endif

/* ── Entry point ───────────────────────────────────────────────────────────
 */
void App_Run(void) {
  HAL_Delay(500); /* let peripherals settle */

#if RUN_MODE == RUN_MODE_TEST_CLI

  TestCLI_Init(&huart3);
  TestCLI_Run(); /* never returns */
#elif RUN_MODE == RUN_MODE_TEST_STATE

  CommandDispatcher_t* dispatcher = Sys_GetCommandDispatcher();
  StateMachine_Init(dispatcher);

  bool data_injected = false;

  while (1) {
    StateMachine_Update();

    if (Interrupt_GetState() == IS_ESTOP) {
      return;
    }

    if (StateMachine_IsIdle() && !data_injected) {
      test_cmd.pieces_count = 2;

      /* Piece 1 */
      test_cmd.pieces[0].piece_id = 1;
      test_cmd.pieces[0].pick_x = 30.0f;
      test_cmd.pieces[0].pick_y = 30.0f;
      test_cmd.pieces[0].place_x = 40.0f;
      test_cmd.pieces[0].place_y = 40.0f;
      test_cmd.pieces[0].rotation = 90.0f;

      /* Piece 2 */
      test_cmd.pieces[1].piece_id = 2;
      test_cmd.pieces[1].pick_x = 100.0f;
      test_cmd.pieces[1].pick_y = 100.0f;
      test_cmd.pieces[1].place_x = 180.0f;
      test_cmd.pieces[1].place_y = 200.0f;
      test_cmd.pieces[1].rotation = -45.0f;

      StateMachine_StartManual(&test_cmd);
      data_injected = true;
    }

    if (StateMachine_IsIdle() && data_injected) {
      return;
    }
  }

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