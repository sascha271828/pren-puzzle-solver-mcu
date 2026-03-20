#include "main.h"

#include "sys_init.h"

#include "command_dispatcher.h"
#include "motion_planner.h"
#include "step_generator.h"
#include "stepper.h"
#include "uart_receiver.h"
#include "usart.h"
#include "utils.h"

#include <stddef.h>

/* clang-format off */
/* planer constants */
/*
static const float planner_steps_per_mm_x = CONFIG_CONSTANT_X_STEPS_PER_MM;
static const float planner_steps_per_mm_y = CONFIG_CONSTANT_Y_STEPS_PER_MM;

static const float planner_max_velocity = CONFIG_MAX_VELOCITY_MM_S;
static const float planner_max_acceleration = CONFIG_MAX_ACCELERATION_MM_SS; */

static Stepper_t stepper_x;
static Stepper_t stepper_y;
static Piston_t piston = {
  .piston_1_extend = {
    .port = DOUT_2_GPIO_Port,
    .pin = DOUT_2_Pin,
  },
  .piston_1_retract= {
    .port = DOUT_3_GPIO_Port,
    .pin = DOUT_3_Pin,
  },
  #if CONFIG_PISTON_SEPARAT_PINS
  /* TODO if implemented */
  #endif
};



/* motor x-axis */
static StepperPin_t pins_stepper_x = {
#if CONFIG_FOR_ENABLE_DRIVER
  .enable = { 
    .port = STEPPER_X_ENABLE_GPIO_Port, 
    .pin = STEPPER_X_ENABLE_Pin 
    },
#endif
#if CONFIG_FOR_NSLEEP_DRIVER
  .nsleep = { 
    .port = STEPPER_X_NSLEEP_GPIO_Port, 
    .pin = STEPPER_X_NSLEEP_Pin 
    },
#endif
#if CONFIG_FOR_NFAULT_DRIVER
  .fault = { 
    .port = STEPPER_X_NFAULT_GPIO_Port, 
    .pin = STEPPER_X_NFAULT_Pin 
    },
#endif
  .step = { 
    .port = STEPPER_X_STEP_GPIO_Port, 
    .pin = STEPPER_X_STEP_Pin 
    },
  .dir = { 
    .port = STEPPER_X_DIR_GPIO_Port, 
    .pin = STEPPER_X_DIR_Pin 
    },
  .m0 = { 
    .port = STEPPER_X_M0_GPIO_Port, 
    .pin = STEPPER_X_M0_Pin 
    },
  .m1 = { 
    .port = STEPPER_X_M1_GPIO_Port, 
    .pin = STEPPER_X_M1_Pin 
    },
  /*.limit_switch_min = { 
    .port = TEST_GPIO_GPIO_Port, 
    .pin = TEST_GPIO_Pin 
    },
  .limit_switch_max = { 
    .port = TEST_GPIO_GPIO_Port, 
    .pin = TEST_GPIO_Pin 
}*/
};

/* motor y-axis */
static StepperPin_t pins_stepper_y = {
#if CONFIG_FOR_ENABLE_DRIVER
  .enable = { .port = STEPPER_Y_ENABLE_GPIO_Port, .pin = STEPPER_Y_ENABLE_Pin },
#endif
#if CONFIG_FOR_NSLEEP_DRIVER
  .nsleep = { .port = STEPPER_Y_NSLEEP_GPIO_Port, .pin = STEPPER_Y_NSLEEP_Pin },
#endif
#if CONFIG_FOR_NFAULT_DRIVER
  .fault = { .port = STEPPER_Y_NFAULT_GPIO_Port, .pin = STEPPER_Y_NFAULT_Pin },
#endif
  .step = { .port = STEPPER_Y_STEP_GPIO_Port, .pin = STEPPER_Y_STEP_Pin },
  .dir = { .port = STEPPER_Y_DIR_GPIO_Port, .pin = STEPPER_Y_DIR_Pin },
  .m0 = { .port = STEPPER_Y_M0_GPIO_Port, .pin = STEPPER_Y_M0_Pin },
  .m1 = { .port = STEPPER_Y_M1_GPIO_Port, .pin = STEPPER_Y_M1_Pin },
  /*.limit_switch_min = { .port = TEST_GPIO_GPIO_Port, .pin = TEST_GPIO_Pin },
  .limit_switch_max = { .port = TEST_GPIO_GPIO_Port, .pin = TEST_GPIO_Pin } */
};

/* motor rotation */
/*
static const StepperPin_t pins_stepper_rot = {
#if CONFIG_FOR_ENABLE_DRIVER
  .enable = { 
    .port = STEPPER_ROT_ENABLE_GPIO_Port,
    .pin = STEPPER_ROT_ENABLE_Pin 
    },
#endif
#if CONFIG_FOR_NSLEEP_DRIVER
  .nsleep = { .port = STEPPER_ROT_NSLEEP_GPIO_Port,
              .pin = STEPPER_ROT_NSLEEP_Pin },
#endif
#if CONFIG_FOR_NFAULT_DRIVER
  .fault = { .port = STEPPER_ROT_NFAULT_GPIO_Port,
             .pin = STEPPER_ROT_NFAULT_Pin },
#endif
  .step = { .port = STEPPER_ROT_STEP_GPIO_Port, .pin = STEPPER_ROT_STEP_Pin },
  .dir = { .port = STEPPER_ROT_DIR_GPIO_Port, .pin = STEPPER_ROT_DIR_Pin },
  .m0 = { .port = STEPPER_ROT_M0_GPIO_Port, .pin = STEPPER_ROT_M0_Pin },
  .m1 = { .port = STEPPER_ROT_M1_GPIO_Port, .pin = STEPPER_ROT_M1_Pin },
  .limit_switch_min = { .port = TEST_GPIO_GPIO_Port, .pin = TEST_GPIO_Pin },
  .limit_switch_max = { .port = TEST_GPIO_GPIO_Port, .pin = TEST_GPIO_Pin }
};
*/

static UartReceiver_t uart_receiver;
static CommandDispatcher_t command_dispatcher;

/* clang-format on */

void Sys_Init(void) {
  /* --- STEPPER INIT --- */
  Stepper_Init(&stepper_x, pins_stepper_x);
  Stepper_Init(&stepper_y, pins_stepper_y);

  Piston_Init(&piston);

  StepGenerator_Init(&stepper_x, &stepper_y);

  /* --- UART / COMMUNICATION INIT --- */
  UartReceiver_Init(&uart_receiver, &huart7);
  CommandDispatcher_Init(&command_dispatcher, &uart_receiver);
  UartReceiver_Start(&uart_receiver);
}

Piston_t* Sys_GetPiston(void) { return &piston; }

UartReceiver_t* Sys_GetUartReceiver(void) { return &uart_receiver; }

CommandDispatcher_t* Sys_GetCommandDispatcher(void) {
  return &command_dispatcher;
}
