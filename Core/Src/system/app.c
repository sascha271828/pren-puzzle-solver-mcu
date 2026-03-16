#include "main.h"

#include "app.h"

#include "piston.h"
#include "step_generator.h"
#include "sys_config.h"
#include "sys_init.h"

#define TEST_STEPPER (1)
#define TEST_PISTON (0)

/*static StateMachine_t* machine;*/
volatile uint32_t system_tick = 0;
volatile bool piston_moving = false;

void App_Run(void) {
  HAL_Delay(100);
#if TEST_STEPPER
  // Create a move: 200 steps X positive, 100 steps Y positive
  MoveBlock_t test_block;
  test_block.steps_x = 200;
  test_block.steps_y = 100;
  test_block.path_steps = 200;  // max(200,100) = 200
  // Constant speed: choose interval in ticks (100 kHz tick)
  // e.g., 1000 ticks between steps → 100 steps/second
  test_block.cruise_interval = 1000;
  test_block.initial_interval = 1000;  // same for no accel
  test_block.accel_until = 0;          // no acceleration phase
  test_block.decel_at =
      200;  // decel starts at last step? Actually with constant speed we can
            // set decel_at = path_steps to skip decel phase.

  // Set motor directions based on step signs
  // Stepper_SetDirection(&stepper_x, test_block.steps_x > 0);
  // Stepper_SetDirection(&stepper_y, test_block.steps_y > 0);

  if (StepGenerator_AddBlock(&test_block)) {
    // Wait for completion
    while (StepGenerator_IsBusy()) {
      // You could add a small delay or just spin
    }
  }
  for (;;) {
    HAL_Delay(500);
    // HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin); // assuming LD2 is the user
    // LED
  }

#endif
#if TEST_PISTON
  HAL_Delay(500);
  Piston_t* piston = Sys_GetPiston();
  Piston_Set(piston, PISTON_START_POS);
  HAL_Delay(5000);
  for (;;) {
    Piston_Set(piston, PISTON_MOVE_POS);
    HAL_Delay(5000);
    Piston_Set(piston, PISTON_GRAB);
    HAL_Delay(5000);
    Piston_Set(piston, PISTON_MOVE_POS);
    HAL_Delay(5000);
    Piston_Set(piston, PISTON_RELEASE);
    HAL_Delay(5000);
  }

#endif
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim) {
  if (htim->Instance == TIM2) {
    system_tick++;
    StepGenerator_Update();
  }
  if (htim->Instance == TIM3) {
    piston_moving = false;
  }
}
