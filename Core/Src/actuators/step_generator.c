#include "main.h"

#include "step_generator.h"

#include "sys_config.h"
#include "tim.h"

#include <stdbool.h>
#include <stdlib.h>

/* ========================
 *   PRIVATE DECLARATION
 * ======================== */

typedef struct {
  uint32_t interval[AXIS_ACCEL_STEPS_IDEAL];
} interval_table_t;

typedef struct {
  const volatile MoveBlock_t* block;
  volatile int32_t dda_counter;
  volatile uint32_t step_index;
  volatile uint32_t current_interval;
  volatile uint32_t ticks_until_next;
  volatile uint32_t steps_minor;
} MoveExec_t;

/* ========================
 *   PRIVATE VARIABLES
 * ======================== */

static interval_table_t StepGenerator_Table;
static MoveExec_t current_block;

static Stepper_t* motor_x = NULL;
static Stepper_t* motor_y = NULL;

/* ========================
 *   PRIVATE FUNCTIONS
 * ======================== */

static void StepGenerator_BuildRampTable(void) {
  float c = (float)TIMER_FREQ_HZ_ACTUATORS * sqrtf(2.0f / (float)AXIS_ACCEL_STEPS_S2);

  StepGenerator_Table.interval[0] = (uint32_t)(c + 0.5f); /* gerundet, nicht truncated */

  for (size_t k = 1; k < AXIS_ACCEL_STEPS_IDEAL; k++) {
    c = c - (2.0f * c) / (4.0f * k + 1);
    uint32_t interval = (uint32_t)(c + 0.5f);
    if (interval <= AXIS_CRUISE_INTERVAL) {
      interval = AXIS_CRUISE_INTERVAL;
    }
    StepGenerator_Table.interval[k] = interval;
  }
}

/* ========================
 *   PUBLIC API
 * ======================== */

void StepGenerator_Init(Stepper_t* mx, Stepper_t* my) {
  motor_x = mx;
  motor_y = my;
  StepGenerator_BuildRampTable();
}

MoveBlock_t StepGenerator_GenerateBlock(int32_t steps_x, int32_t steps_y) {
  uint32_t abs_x = abs(steps_x);
  uint32_t abs_y = abs(steps_y);
  bool x_dom = abs_x > abs_y;
  uint32_t path_steps_uint = x_dom ? abs_x : abs_y;

  uint32_t accel_steps = path_steps_uint / 2;
  uint32_t decel_at;

  if (path_steps_uint >= 2 * AXIS_ACCEL_STEPS_IDEAL) {
    /* Trapezoid */
    accel_steps = AXIS_ACCEL_STEPS_IDEAL;
    decel_at = path_steps_uint - AXIS_ACCEL_STEPS_IDEAL;
  } else {
    /* Triangle: keine Cruise-Phase */
    accel_steps = path_steps_uint / 2;
    decel_at = accel_steps;
  }

  MoveBlock_t newBlock = { .steps_x = steps_x,
                           .steps_y = steps_y,
                           .accel_until = accel_steps,
                           .decel_at = decel_at,
                           .cruise_interval = AXIS_CRUISE_INTERVAL,
                           .path_steps = path_steps_uint,
                           .table_len = accel_steps,
                           .x_dominant = x_dom };
  return newBlock;
}

bool StepGenerator_StartMove(const MoveBlock_t* block) {
  if (StepGenerator_IsBusy()) {
    return false;
  }
  current_block.block = block;
  current_block.dda_counter = -(int32_t)(block->path_steps / 2);
  current_block.ticks_until_next = 0;
  current_block.step_index = 0;
  current_block.steps_minor = block->x_dominant ? abs(block->steps_y) : abs(block->steps_x);

  /* direction */
  Stepper_Enable(motor_x, true);
  Stepper_Enable(motor_y, true);
  Stepper_SetDirection(motor_x, (current_block.block->steps_x > 0));
  Stepper_SetDirection(motor_y, (current_block.block->steps_y > 0));

  HAL_TIM_Base_Start_IT(&htim2);
  return true;
}

void StepGenerator_Update(void) {
  if (current_block.block == NULL) return;

  /* clear prior steps */
  if (motor_x->pulse_active) { /* TODO: remove condition because not needed */
    Stepper_ClearStep(motor_x);
  }
  if (motor_y->pulse_active) {
    Stepper_ClearStep(motor_y);
  }

  /* Decide if need to step */
  if (current_block.ticks_until_next == 0) {
    if (current_block.step_index >= (current_block.block->path_steps)) {
      current_block.block = NULL;
      Stepper_ClearStep(motor_x);
      Stepper_ClearStep(motor_y);
      Stepper_Enable(motor_x, false);
      Stepper_Enable(motor_y, false);
      return;
    }
    Stepper_SetStep(current_block.block->x_dominant ? motor_x : motor_y); /* major motor */

    current_block.dda_counter += current_block.steps_minor;
    if (current_block.dda_counter >= 0) {
      Stepper_SetStep(current_block.block->x_dominant ? motor_y : motor_x); /* minor motor */
      current_block.dda_counter -= current_block.block->path_steps;
    }

    /* - determine next interval - */
    if (current_block.step_index < current_block.block->accel_until) {
      current_block.current_interval = StepGenerator_Table.interval[current_block.step_index];

    } else if (current_block.step_index < current_block.block->decel_at) {
      current_block.current_interval = current_block.block->cruise_interval;
    } else {
      uint32_t decel_steps_done = current_block.step_index - current_block.block->decel_at;

      if (decel_steps_done >= current_block.block->table_len) {
        current_block.current_interval = StepGenerator_Table.interval[0];
      } else {
        uint32_t mirror = current_block.block->table_len - decel_steps_done - 1;
        current_block.current_interval = StepGenerator_Table.interval[mirror];
      }
    }

    /* - update block - */
    current_block.ticks_until_next = current_block.current_interval - 1;
    current_block.step_index++;
  } else {
    current_block.ticks_until_next--;
  }
}

bool StepGenerator_IsBusy(void) { return (current_block.block != NULL); }

void StepGenerator_Abort(void) {
  Stepper_ClearStep(motor_x);
  Stepper_ClearStep(motor_y);
  Stepper_Enable(motor_x, false);
  Stepper_Enable(motor_y, false);

  current_block.block = NULL;
}