#include "main.h"
#include "app.h"
#include "command_dispatcher.h"
#include "piston.h"
#include "rotator.h"
#include "magnet.h"
#include "step_generator.h"
#include "sys_config.h"
#include "sys_init.h"
#include "uart_receiver.h"
#include "state_machine.h"

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
    StepGenerator_StartMove(&test_block_positive);
    while (StepGenerator_IsBusy()) {
    }
    HAL_Delay(5000);

    StepGenerator_StartMove(&test_block_negative);
    while (StepGenerator_IsBusy()) {
    }
    HAL_Delay(20000);
  }
#elif TEST_PISTON
#else

  CommandDispatcher_t* dispatcher = Sys_GetCommandDispatcher();
  Piston_t* piston = Sys_GetPiston();
  Magnet_t* magnet = Sys_GetMagnet();
  Rotator_t* rotator = Sys_GetRotator();

  StateMachine_Init(dispatcher, magnet);

  Piston_Set(piston, PISTON_POS_START);
  Magnet_SetState(magnet, false);

  for (;;) {
    CommandDispatcher_Poll(dispatcher);
    StateMachine_Update();
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

void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart) {
  if (huart->Instance == UART7) {
    UartReceiver_RxCallback(Sys_GetUartReceiver());
  }
}