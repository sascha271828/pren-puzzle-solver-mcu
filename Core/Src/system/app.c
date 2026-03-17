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

  MoveBlock_t test_block =
      StepGenerator_GenerateBlock(200000, 10000, 0, 180000, 100, 100);

  for (;;) {
    StepGenerator_StartStep(&test_block);
    while (StepGenerator_IsBusy());
    HAL_Delay(500);
  }

#endif
#if TEST_PISTON
  HAL_Delay(500);
  Piston_t* piston = Sys_GetPiston();

  // Drive to a known start state first
  Piston_Set(piston, PISTON_POS_START);
  HAL_Delay(1000);

  for (;;) {
    PistonResult_e r;

    r = Piston_Set(piston, PISTON_POS_MOVE);
    // assert(r == PISTON_OK);
    HAL_Delay(1000);

    r = Piston_Set(piston, PISTON_POS_GRAB);
    // assert(r == PISTON_OK);
    HAL_Delay(1000);

    r = Piston_Set(piston, PISTON_POS_MOVE);  // GRAB → MOVE (defined)
    // assert(r == PISTON_OK);
    HAL_Delay(1000);

    r = Piston_Set(piston, PISTON_POS_RELEASE);
    // assert(r == PISTON_OK);
    HAL_Delay(1000);

    r = Piston_Set(piston, PISTON_POS_MOVE);  // RELEASE → MOVE (defined)
    // assert(r == PISTON_OK);
    HAL_Delay(1000);
  }
#endif
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim) {
  if (htim->Instance == TIM2) {
    system_tick++;
    StepGenerator_Update();
  }
}
