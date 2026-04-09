#include "main.h"
#include "app.h"
#include "command_dispatcher.h"
#include "uart_receiver.h"
#include "state_machine.h"
#include "magnet.h"
#include "piston.h"
#include "rotator.h"
#include "step_generator.h"
#include "sys_config.h"
#include "sys_init.h"

#define RUN_HARDWARE_TESTS 1

volatile uint32_t system_tick = 0;

#if RUN_HARDWARE_TESTS

/* ── Test parameters ────────────────────────────────────────────────────── */
#define TEST_STEPS_MAJOR 2000
#define TEST_STEPS_MINOR 1000
#define TEST_ROT_STEPS 1000
#define TEST_PAUSE_MS 2000

/* ── Helpers ─────────────────────────────────────────────────────────────── */
static void wait_step_generator(void) {
  while (StepGenerator_IsBusy()) {
  }
}

static void wait_rotator(void) {
  while (Rotator_IsBusy()) {
  }
}

static void wait_piston(void) {
  while (Piston_IsBusy()) {
  }
}

/* ── Sub-tests ───────────────────────────────────────────────────────────── */

static void test_step_generator(void) {
  /* X-dominant diagonal move */
  MoveBlock_t fwd =
      StepGenerator_GenerateBlock(TEST_STEPS_MAJOR, TEST_STEPS_MINOR);
  MoveBlock_t rev =
      StepGenerator_GenerateBlock(-TEST_STEPS_MAJOR, -TEST_STEPS_MINOR);

  StepGenerator_StartMove(&fwd);
  wait_step_generator();
  HAL_Delay(TEST_PAUSE_MS);

  StepGenerator_StartMove(&rev);
  wait_step_generator();
  HAL_Delay(TEST_PAUSE_MS);

  /* Y-dominant diagonal move */
  MoveBlock_t fwd_y =
      StepGenerator_GenerateBlock(TEST_STEPS_MINOR, TEST_STEPS_MAJOR);
  MoveBlock_t rev_y =
      StepGenerator_GenerateBlock(-TEST_STEPS_MINOR, -TEST_STEPS_MAJOR);

  StepGenerator_StartMove(&fwd_y);
  wait_step_generator();
  HAL_Delay(TEST_PAUSE_MS);

  StepGenerator_StartMove(&rev_y);
  wait_step_generator();
  HAL_Delay(TEST_PAUSE_MS);
}

static void test_rotator(void) {
  RotateBlock_t fwd = Rotator_GenerateBlock(TEST_ROT_STEPS);
  RotateBlock_t rev = Rotator_GenerateBlock(-TEST_ROT_STEPS);

  Rotator_StartMove(&fwd);
  wait_rotator();
  HAL_Delay(TEST_PAUSE_MS);

  Rotator_StartMove(&rev);
  wait_rotator();
  HAL_Delay(TEST_PAUSE_MS);
}

static void test_piston(void) {
  /* START → MOVE → GRAB → MOVE → RELEASE → MOVE → START */

  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);

  HAL_Delay(TEST_PAUSE_MS);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);

  HAL_Delay(TEST_PAUSE_MS);
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

/* ── Entry point ─────────────────────────────────────────────────────────── */
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

  StateMachine_Init(dispatcher);

  Piston_Set(PISTON_POS_START);
  Magnet_SetState(false);

  for (;;) {
    CommandDispatcher_Poll(dispatcher);
    StateMachine_Update();
  }
#endif
}

/* ── ISR ─────────────────────────────────────────────────────────────────── */
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