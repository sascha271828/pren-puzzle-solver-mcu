#include "sys_init.h"

#include "actuator.h"
#include "end_effector.h"
#include "main.h"
#include "motion_planer.h"
#include "piston.h"
#include "state_machine.h"
#include "step_generator.h"
#include "stepper.h"
#include "sys_config.h"

/* clang-format off */
/* planer constants */
static const float planner_steps_per_mm_x = 0;
static const float planner_steps_per_mm_y = 0;

static const float planner_max_velocity = 0;
static const float planner_max_acceleration = 0;

static const* Stepper_t stepper_x = NULL;
static const* Stepper_t stepper_y = NULL;

/* motor x-axis */
static const StepperPin_t pins_stepper_x = {
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
  .limit_switch_min = { 
    .port = TEST_GPIO_GPIO_Port, 
    .pin = TEST_GPIO_Pin 
    },
  .limit_switch_max = { 
    .port = TEST_GPIO_GPIO_Port, 
    .pin = TEST_GPIO_Pin 
}
};

/* motor y-axis */
static const StepperPin_t pins_stepper_y = {
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
  .limit_switch_min = { .port = TEST_GPIO_GPIO_Port, .pin = TEST_GPIO_Pin },
  .limit_switch_max = { .port = TEST_GPIO_GPIO_Port, .pin = TEST_GPIO_Pin }
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
  .limit_switch_min = { .port = TEST_GPIO_GPIO_Port, .pin = TEST_GPIO_Pin },
  .limit_switch_max = { .port = TEST_GPIO_GPIO_Port, .pin = TEST_GPIO_Pin }
};

/* piston */
static Piston_t piston = {
  .state = PISTON_STOP,
  .movement_time = 0,
  .piston_1_high = { 
    .port = TEST_GPIO_GPIO_Port, 
    .pin = TEST_GPIO_Pin 
    },
  .piston_1_low = { 
    .port = TEST_GPIO_GPIO_Port, 
    .pin = TEST_GPIO_Pin 
    },
  .piston_2_high = { 
    .port = TEST_GPIO_GPIO_Port, 
    .pin = TEST_GPIO_Pin 
    },
  .piston_2_low = { 
    .port = TEST_GPIO_GPIO_Port, 
    .pin = TEST_GPIO_Pin 
    },
#if CONFIG_PISTON_HAS_LIMIT_SWITCH
  .limit_switch_contracted = { 
    .port = TEST_GPIO_GPIO_Port,
    .pin = TEST_GPIO_Pin 
    },
  .limit_switch_extended = { 
    .port = TEST_GPIO_GPIO_Port,              
    .pin = TEST_GPIO_Pin 
    },
#endif
  .piston_high_active = false,
  .piston_low_active = false
};

/* magnet */
static BinaryActuator_t magnet = { 
    .pin = { 
        .port = TEST_GPIO_GPIO_Port,
        .pin = TEST_GPIO_Pin 
    },
    .state = ACTUATOR_OFF,
    .active_low = false 
};

/* clang-format on */

void Sys_Init(void) {
  /* --- STEPPER INIT --- */
  Stepper_Init(&stepper_x, pins_stepper_x);
  Stepper_Init(&stepper_y, pins_stepper_y);

  Stepper_Init(&stepper_rot, pins_stepper_rot);

  StepGenerator_Init(&stepper_x, &stepper_y);

  /* --- PLANER INIT --- */

  /* --- TOOL INIT --- */
  // Tool_Init(&tool, &magnet, &piston, &stepper_rot);

  /* --- MACHINE INIT --- */
  // Machine_Init(&machine, &planner, &tool);
}

StateMachine_t* Sys_GetMachine(void) { return &machine; }
