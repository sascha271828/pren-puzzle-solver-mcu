#include "main.h"

#include "app.h"

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

static void test_piston(void) {
  /* START → MOVE → GRAB → MOVE → RELEASE → MOVE → START */

  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);

  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);

  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
}

/* ── Entry point ───────────────────────────────────────────────────────────
 */
void App_Run(void) {
  HAL_Delay(500); /* let peripherals settle */

  for (;;) {
  }
}

/* ── ISR ───────────────────────────────────────────────────────────────────
 */
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