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

volatile uint32_t system_tick = 0;

/* ── Entry point ───────────────────────────────────────────────────────────
 */
void App_Run(void) {
  HAL_Delay(500); /* let peripherals settle */

#if RUN_MODE == RUN_MODE_TEST_CLI
  TestCLI_Init(&huart3);
  TestCLI_Run(); /* never returns */

#elif RUN_MODE == RUN_MODE_APP
  /* --- STATE MACHINE MODUS --- */
  CommandDispatcher_t* dispatcher = Sys_GetCommandDispatcher();
  Magnet_t* magnet = Sys_GetMagnet();

  StateMachine_Init(dispatcher, magnet);

  Piston_Set(PISTON_POS_START);
  Magnet_SetState(magnet, false);

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

/* TODO: vlt. längsämeres Polling für gewisse Teile? */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim) {
  if (htim->Instance == TIM2) {
    system_tick++;
    EmergencyStop_Process(); /* check if emergency pressed -- if pressed,
                              g_int_state gets set to IS_ESTOP inside of the
                              function */
    Buttons_Poll_ISR();
    switch (Interrupt_GetState()) {
      case IS_HOMING:
        Homer_Update();
        break;
      case IS_RUNNING:
      case IS_READY:
        if (LimitSwitch_Activated()) {
          StepGenerator_Abort();
        }
        StepGenerator_Update();
        Rotator_Update();
        Piston_Update();
        break;
      case IS_ESTOP:
        Magnet_SetState(false);
        StepGenerator_Abort();
        Rotator_Abort();
        Piston_Abort();
        break;
      case IS_INIT:
        Piston_Update();
      default:
        break;
    }
  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart) {
  if (huart->Instance == UART7) {
    UartReceiver_RxCallback(Sys_GetUartReceiver());
  }
}