#include "main.h"

#include "app.h"

#include "step_generator.h"
#include "sys_config.h"

/*static StateMachine_t* machine;*/
volatile uint32_t system_tick = 0;

void App_Run(void) {
  HAL_Delay(100);

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
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim->Instance == TIM2) {
    system_tick++;
    StepGenerator_Update();
  }
}
