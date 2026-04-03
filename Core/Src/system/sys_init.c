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
  .enable = { 
    .port = STEPPER_X_ENABLE_GPIO_Port, 
    .pin = STEPPER_X_ENABLE_Pin 
    },
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
.limit_switch_min = { 
    .port = DIN_1_GPIO_Port, 
    .pin = DIN_1_Pin 
    },
  .limit_switch_max = { 
    .port = DIN_2_GPIO_Port, 
    .pin = DIN_2_Pin 
    },
};

/* motor y-axis */
static StepperPin_t pins_stepper_y = {
  .enable = { .port = STEPPER_Y_ENABLE_GPIO_Port, .pin = STEPPER_Y_ENABLE_Pin },
#if CONFIG_FOR_NSLEEP_DRIVER
  .nsleep = { .port = STEPPER_Y_NSLEEP_GPIO_Port, .pin = STEPPER_Y_NSLEEP_Pin },
#endif
#if CONFIG_FOR_NFAULT_DRIVER
  .fault = { .port = STEPPER_Y_NFAULT_GPIO_Port, .pin = STEPPER_Y_NFAULT_Pin },
#endif
  .step = { 
    .port = STEPPER_Y_STEP_GPIO_Port, 
    .pin = STEPPER_Y_STEP_Pin 
  },
  .dir = { 
    .port = STEPPER_Y_DIR_GPIO_Port, 
    .pin = STEPPER_Y_DIR_Pin 
  },
  .m0 = { 
    .port = STEPPER_Y_M0_GPIO_Port, 
    .pin = STEPPER_Y_M0_Pin 
  },
  .m1 = { 
    .port = STEPPER_Y_M1_GPIO_Port, 
    .pin = STEPPER_Y_M1_Pin 
  },
  .limit_switch_min = { 
    .port = DIN_3_GPIO_Port, 
    .pin = DIN_3_Pin 
    },
  .limit_switch_max = { 
    .port = DIN_4_GPIO_Port, 
    .pin = DIN_4_Pin 
    },

};

/* motor rotation */
static const StepperPin_t pins_stepper_rot = {
  .enable = { 
    .port = STEPPER_ROT_ENABLE_GPIO_Port,
    .pin = STEPPER_ROT_ENABLE_Pin 
    },
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
  /* --- STEPPER --- */
  Stepper_Init(&stepper_x, pins_stepper_x, CONFIG_AXIS_MICRO, true);
  Stepper_Init(&stepper_y, pins_stepper_y, CONFIG_AXIS_MICRO, true);
  Stepper_Init(&stepper_rot, pins_stepper_rot, CONFIG_ROT_MICRO, true);
  Stepper_Enable(&stepper_x, true);
  Stepper_Enable(&stepper_y, true);
  Stepper_Enable(&stepper_rot, true);

  /* to get the steppers into a set position (block unwanted movement)*/
  Stepper_SetStep(&stepper_x);
  Stepper_SetStep(&stepper_y);
  Stepper_SetStep(&stepper_rot);
  HAL_Delay(1);
  Stepper_ClearStep(&stepper_x);
  Stepper_ClearStep(&stepper_y);
  Stepper_ClearStep(&stepper_rot);

  /* --- PISTON --- */
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
  Piston_Init(piston_1_extend, piston_1_retract);
  /*  while (true) {
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);
      HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);

      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);
      HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);

  HAL_GPIO_WritePin(piston_1_extend.port, piston_1_extend.pin, GPIO_PIN_SET);

  HAL_GPIO_WritePin(
      piston_1_retract.port, piston_1_retract.pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(
      piston_1_extend.port, piston_1_extend.pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(
      piston_1_retract.port, piston_1_retract.pin, GPIO_PIN_RESET);
      */
  //}
  /* --- SECOND LAYER --- */
  StepGenerator_Init(&stepper_x, &stepper_y);
  Rotator_Init(&stepper_rot);
  Magnet_Init(magnet_pin);

  /* --- UART / COMMUNICATION INIT --- */
  UartReceiver_Init(&uart_receiver, &huart5);
  CommandDispatcher_Init(&command_dispatcher, &uart_receiver);
  UartReceiver_Start(&uart_receiver);
}

UartReceiver_t* Sys_GetUartReceiver(void) { return &uart_receiver; }

CommandDispatcher_t* Sys_GetCommandDispatcher(void) {
  return &command_dispatcher;
}
