#include "main.h"

#include "app.h"

#include "buttons.h"
#include "command_dispatcher.h"
#include "emergency_stop.h"
#include "homer.h"
#include "interrupt.h"
#include "leds.h"
#include "limit_switch.h"
#include "magnet.h"
#include "motion_planner.h"
#include "piston.h"
#include "rotator.h"
#include "state_machine.h"
#include "status_leds.h"
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

void App_Run(void) {
  HAL_Delay(CONFIG_INIT_WAIT_PERIPHERALS); /* let peripherals settle */


#if RUN_MODE == RUN_MODE_TEST_CLI
  /* --- TEST MODE STATE MACHINE --- */
  TestCLI_Init(&huart3);
  // TestCLI_Init(&huart2);
  TestCLI_Run();
#elif RUN_MODE == RUN_MODE_TEST_STATE
  /* --- TEST MODE STATE MACHINE --- */
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
  /* --- PRODUCTION MODE --- */
  CommandDispatcher_t* dispatcher = Sys_GetCommandDispatcher();
  StateMachine_Init(dispatcher);

  for (;;) {
    CommandDispatcher_Poll(dispatcher);
    StateMachine_Update();
  }

#elif RUN_MODE == RUN_MODE_LED
  while (1) {
    if (Buttons_Start_Pressed()) {
      Leds_Set(true);
      Buttons_Start_RearmPressDetection();
    }

    if (Buttons_Reset_Pressed()) {
      Leds_Set(false);
      Buttons_Reset_RearmPressDetection();
    }
  }

#else
#error "RUN_MODE not defined or unknown — set it in sys_config.h"
#endif
}

/* ========================
 *   ISR
 * ======================== */

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim) {
  if (htim->Instance == TIM2) {
#if RUN_MODE == RUN_MODE_LED
    Buttons_Poll_ISR();
#else
#if TEST_ISR_TIME
    HAL_GPIO_WritePin(DOUT_8_GPIO_Port, DOUT_8_Pin, GPIO_PIN_SET);
#endif
    EmergencyStop_Process();
    Buttons_Poll_ISR();
    StatusLeds_Blink_ISR();

    switch (Interrupt_GetState()) {
      case IS_HOMING:
        Homer_Update();
        Piston_Update();
        Rotator_Update();
        break;
      case IS_READY:
      case IS_RUNNING:
        if (LimitSwitch_Activated()) {
          Interrupt_SetState(IS_ESTOP);
          break;
        }
        StepGenerator_Update();
        Piston_Update();
        Rotator_Update();
        Magnet_Process();
        break;
      case IS_ESTOP:
        Magnet_SetState(false);
        StepGenerator_Abort();
        Rotator_Abort();
        Piston_Abort();
        Leds_Set(false);
        MotionPlanner_Init();
        if (EmergencyStop_IsActivated() == false) {
          StatusLeds_On(STATUSLED_RED);
          if (Buttons_Reset_Pressed() == true) {
            StatusLeds_Off(STATUSLED_RED);
            Homer_HomingStart();
            Piston_Set(PISTON_POS_START);
            Rotator_ReturnStart();
          }
        }
        break;
      default:
        break;
    }
#if TEST_ISR_TIME
    HAL_GPIO_WritePin(DOUT_8_GPIO_Port, DOUT_8_Pin, GPIO_PIN_RESET);
#endif
#endif
  }
}

/* ========================
 *   UART CALLBACK
 * ======================== */

#if RUN_MODE == RUN_MODE_APP
void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart) {
  if (huart->Instance == USART2) {
    UartReceiver_RxCallback(Sys_GetUartReceiver());
  }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef* huart) {
  if (huart->Instance == USART2) {
    UartReceiver_Start(Sys_GetUartReceiver());
  }
}
#endif