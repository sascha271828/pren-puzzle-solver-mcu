#include "main.h"
#include "app.h"
#include "command_dispatcher.h"
#include "uart_receiver.h"
#include "state_machine.h"
#include "buttons.h"
#include "emergency_stop.h"
#include "homer.h"
#include "interrupt.h"
#include "limit_switch.h"
#include "magnet.h"
#include "piston.h"
#include "rotator.h"
#include "step_generator.h"
#include "sys_config.h"
#include "sys_init.h"

#define RUN_HARDWARE_TESTS 1

volatile uint32_t system_tick = 0;

#if RUN_HARDWARE_TESTS

static void test_piston(void) {
  /* START → MOVE → GRAB → MOVE → RELEASE → MOVE → START */

  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);

  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);

  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
  /*
    Piston_Set(PISTON_POS_START);
    wait_piston();
    HAL_Delay(TEST_PAUSE_MS);

    Piston_Set(PISTON_POS_MOVE);
    wait_piston();
    HAL_Delay(TEST_PAUSE_MS);

    Piston_Set(PISTON_POS_GRAB);
    wait_piston();
    HAL_Delay(TEST_PAUSE_MS);

    Piston_Set(PISTON_POS_MOVE);
    wait_piston();
    HAL_Delay(TEST_PAUSE_MS);

    Piston_Set(PISTON_POS_RELEASE);
    wait_piston();
    HAL_Delay(TEST_PAUSE_MS);

    Piston_Set(PISTON_POS_MOVE);
    wait_piston();
    HAL_Delay(TEST_PAUSE_MS);

    Piston_Set(PISTON_POS_START);
    wait_piston();
    HAL_Delay(TEST_PAUSE_MS);
    */
}

static void test_magnet(void) {
  Magnet_SetState(true);
  HAL_Delay(1000);
  Magnet_SetState(false);
  HAL_Delay(1000);
  Magnet_SetState(true);
  HAL_Delay(1000);
  Magnet_SetState(false);
  HAL_Delay(TEST_PAUSE_MS);
}

#endif /* RUN_HARDWARE_TESTS */

/* ── Entry point ───────────────────────────────────────────────────────────
 */
void App_Run(void) {
  HAL_Delay(500); /* let peripherals settle */

#if RUN_HARDWARE_TESTS
  /* --- TEST MODUS --- */
  for (;;) {
    test_step_generator();
    test_rotator();
    test_piston();
    test_magnet();

    HAL_Delay(TEST_PAUSE_MS);
  }
#else
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
#endif

}

/* ── ISR ───────────────────────────────────────────────────────────────────
 */

/* TODO: vlt. längsämeres Polling für gewisse Teile? */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim) {
  if (htim->Instance == TIM2) {
    EmergencyStop_Process(); /* check if emergency pressed -- if pressed,
                              g_int_state gets set to IS_ESTOP inside of the
                              function */
    Buttons_Poll_ISR();
    switch (Interrupt_GetState()) {
      case IS_HOMING:
        Homer_Update();
        break;
      case IS_RUNNING:
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
      case IS_READY:
      case IS_INIT:
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