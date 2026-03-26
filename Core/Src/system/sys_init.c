#include "main.h"

#include "sys_init.h"

#include "command_dispatcher.h"
#include "magnet.h"
#include "motion_planner.h"
#include "rotator.h"
#include "step_generator.h"
#include "stepper.h"
#include "uart_receiver.h"
#include "usart.h"
#include "utils.h"

#include <stddef.h>

/* clang-format off */
/* planer constants */

static Stepper_t stepper_x;
static Stepper_t stepper_y;
static Stepper_t stepper_rot;


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
/*  .limit_switch_min = { .port = TEST_GPIO_GPIO_Port, .pin = TEST_GPIO_Pin },
  .limit_switch_max = { .port = TEST_GPIO_GPIO_Port, .pin = TEST_GPIO_Pin }*/
};

static UartReceiver_t uart_receiver;
static CommandDispatcher_t command_dispatcher;

/* clang-format on */

void Sys_Init(void) {
  /* --- STEPPER INIT --- */
  Stepper_Init(&stepper_x, pins_stepper_x);
  Stepper_Init(&stepper_y, pins_stepper_y);
  Stepper_Init(&stepper_rot, pins_stepper_rot);
  GPIO_Pin_t piston_1_extend = {
    .port = DOUT_5_GPIO_Port,
    .pin = DOUT_5_Pin,
  };

  GPIO_Pin_t piston_1_retract = {
    .port = DOUT_6_GPIO_Port,
    .pin = DOUT_6_Pin,
  };

  GPIO_Pin_t magnet_pin = {
    .port = DOUT_1_GPIO_Port,
    .pin = DOUT_1_Pin,
  };

  /* --- ACTUATORS --- */
  Piston_Init(piston_1_extend, piston_1_retract);
  StepGenerator_Init(&stepper_x, &stepper_y);
  Rotator_Init(&stepper_rot);
  Magnet_Init(magnet_pin);

  /* --- UART / COMMUNICATION INIT --- */
  UartReceiver_Init(&uart_receiver, &huart7);
  CommandDispatcher_Init(&command_dispatcher, &uart_receiver);
  UartReceiver_Start(&uart_receiver);
}

UartReceiver_t* Sys_GetUartReceiver(void) { return &uart_receiver; }

CommandDispatcher_t* Sys_GetCommandDispatcher(void) {
  return &command_dispatcher;
}
