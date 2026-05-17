#include "main.h"

#include "rotator.h"

#include "math.h"
#include "sys_config.h"
#include "tim.h"

#include <stdbool.h>
#include <stdlib.h>

/* ========================
 *   PRIVATE DECLARATION
 * ======================== */

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

typedef struct {
  uint32_t interval[ROT_ACCEL_STEPS_IDEAL];
} interval_table_rot_t;

/* ========================
 *   PRIVATE VARIABLES
 * ======================== */

static MoveExec_t current_block;
static RotateBlock_t home_block;
static interval_table_rot_t rotator_table;

static Rotator_t rotator = { .motor_rot = NULL, .current_position = 0 };

/* ========================
 *   PRIVATE FUNCTIONS
 * ======================== */

static void Rotator_BuildRampTable(void) {
  float c = (float)TIMER_FREQ_HZ_ACTUATORS * sqrtf(2.0f / (float)ROT_ACCEL_STEPS_S2);

  rotator_table.interval[0] = (uint32_t)(c + 0.5f);

  for (size_t k = 1; k < ROT_ACCEL_STEPS_IDEAL; k++) {
    c = c - (2.0f * c) / (4.0f * k + 1);
    uint32_t interval = (uint32_t)(c + 0.5f);
    if (interval <= ROT_CRUISE_INTERVAL) {
      interval = ROT_CRUISE_INTERVAL;
    }
    rotator_table.interval[k] = interval;
  }
}

/* ========================
 *   PUBLIC API
 * ======================== */

void Rotator_Init(Stepper_t* rot) {
  rotator.motor_rot = rot;
  Rotator_BuildRampTable();
}

RotateBlock_t Rotator_GenerateBlock(int32_t steps) {
  uint32_t path_steps_uint = (uint32_t)(abs(steps));

  uint32_t accel_steps = path_steps_uint / 2;
  uint32_t decel_at;

  if (path_steps_uint >= 2 * ROT_ACCEL_STEPS_IDEAL) {
    accel_steps = ROT_ACCEL_STEPS_IDEAL;
    decel_at = path_steps_uint - ROT_ACCEL_STEPS_IDEAL;
  } else {
    accel_steps = path_steps_uint / 2;
    decel_at = accel_steps;
  }

  RotateBlock_t newBlock = {
    .steps = steps,
    .accel_until = accel_steps,
    .decel_at = decel_at,
    .cruise_interval = ROT_CRUISE_INTERVAL,
    .path_steps = path_steps_uint,
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
  rotator.current_position += current_block.block->steps;

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
      current_block.current_interval = rotator_table.interval[current_block.step_index];

    } else if (current_block.step_index < current_block.block->decel_at) {
      current_block.current_interval = current_block.block->cruise_interval;
    } else {
      uint32_t decel_steps_done = current_block.step_index - current_block.block->decel_at;

      if (decel_steps_done >= current_block.block->table_len) {
        current_block.current_interval = rotator_table.interval[0];
      } else {
        uint32_t mirror = current_block.block->table_len - decel_steps_done - 1;
        current_block.current_interval = rotator_table.interval[mirror];
      }
    }

    /* update block */
    current_block.ticks_until_next = current_block.current_interval - 1;
    current_block.step_index++;
  } else {
    current_block.ticks_until_next--;
  }
}

bool Rotator_IsBusy(void) { return (current_block.block != NULL); }

void Rotator_Abort(void) {
  current_block.block = NULL;
  Stepper_ClearStep(rotator.motor_rot);
  Stepper_Enable(rotator.motor_rot, false);
}
