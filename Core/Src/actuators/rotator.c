#include "main.h"

#include "rotator.h"

#include "math.h"
#include "sys_config.h"
#include "tim.h"

#include <stdbool.h>
#include <stdlib.h>

/* runtime */
typedef struct {
  const RotateBlock_t* block;
  volatile uint32_t step_index;
  volatile uint32_t current_interval;
  volatile uint32_t ticks_until_next;
} MoveExec_t;

typedef struct {
  Stepper_t* motor_rot;
  int32_t current_position;
} Rotator_t;

static MoveExec_t current_block;
static RotateBlock_t home_block;

static Rotator_t rotator = { .motor_rot = NULL, .current_position = 0 };

void Rotator_Init(Stepper_t* rot) { rotator.motor_rot = rot; }

RotateBlock_t Rotator_GenerateBlock(int32_t steps) {
  uint32_t path_steps_uint = (uint32_t)(abs(steps));
  if (path_steps_uint <= 1) {
    path_steps_uint = 2;
  }

  uint32_t accel_steps = path_steps_uint / 2;

  /* trapezoid or triangle profile */
  if (2 * ROT_ACCEL_STEPS_IDEAL < path_steps_uint) {
    accel_steps = ROT_ACCEL_STEPS_IDEAL;
  }

  interval_table_rot_t table;
  float c = TIMER_FREQ_HZ_ACTUATORS * sqrtf(2.0f / ROT_ACCEL_STEPS_S2);
  table.interval[0] = (uint32_t)c;

  for (size_t k = 1; k < accel_steps; k++) {
    c = c - (2.0f * c) / (4.0f * k + 1);
    table.interval[k] = (uint32_t)c;
  }

  RotateBlock_t newBlock = {
    .steps = steps,
    .accel_until = accel_steps,
    .decel_at = path_steps_uint - accel_steps,
    .cruise_interval = ROT_CRUISE_INTERVAL,
    .path_steps = path_steps_uint,
    .interval_table = table,
    .table_len = accel_steps,
  };

  return newBlock;
}

bool Rotator_ReturnStart(void) {
  if (Rotator_IsBusy()) {
    return false;
  }
  home_block = Rotator_GenerateBlock(-rotator.current_position);
  Rotator_StartMove(&home_block);
  return true;
}

bool Rotator_StartMove(const RotateBlock_t* block) {
  if (Rotator_IsBusy()) {
    return false;
  }
  current_block.block = block;
  current_block.ticks_until_next = 0;
  current_block.step_index = 0;

  Stepper_Enable(rotator.motor_rot, true);
  Stepper_SetDirection(rotator.motor_rot, (current_block.block->steps > 0));

  HAL_TIM_Base_Start_IT(&htim2);
  return true;
}

void Rotator_Update(void) {
  if (current_block.block == NULL) return;

  if (rotator.motor_rot->pulse_active) {
    Stepper_ClearStep(rotator.motor_rot);
  }

  /* decide if need to step */
  if (current_block.ticks_until_next == 0) {
    if (current_block.step_index >= current_block.block->path_steps) {
      current_block.block = NULL;
      Stepper_ClearStep(rotator.motor_rot);
      return;
    }

    Stepper_SetStep(rotator.motor_rot);

    if (current_block.step_index < current_block.block->accel_until) {
      current_block.current_interval = current_block.block->interval_table
                                           .interval[current_block.step_index];

    } else if (current_block.step_index < current_block.block->decel_at) {
      current_block.current_interval = current_block.block->cruise_interval;
    } else {
      uint32_t decel_steps_done =
          current_block.step_index - current_block.block->decel_at;

      if (decel_steps_done >= current_block.block->table_len) {
        current_block.current_interval =
            current_block.block->interval_table.interval[0];
      } else {
        uint32_t mirror = current_block.block->table_len - decel_steps_done - 1;
        current_block.current_interval =
            current_block.block->interval_table.interval[mirror];
      }
    }

    /* - update block - */
    current_block.ticks_until_next = current_block.current_interval - 1;
    current_block.step_index++;
  } else {
    current_block.ticks_until_next--;
  }
}

bool Rotator_IsBusy(void) { return (current_block.block != NULL); }

void Rotator_Abort(void) { current_block.block = NULL; }
