#include "main.h"

#include "rotator.h"

#include "math.h"
#include "sys_config.h"
#include "tim.h"

#include <stdbool.h>
#include <stdlib.h>

#define V_MAX_STEPS (CONFIG_MAX_SPEED_ROT * CONFIG_STEPS_PER_01_DEGREE)
#define ACCEL_STEPS_S (CONFIG_ACCEL_AXIS_ROT * CONFIG_STEPS_PER_01_DEGREE)
#define ACCEL_STEPS_IDEAL ((V_MAX_STEPS * V_MAX_STEPS) / (2 * ACCEL_STEPS_S))
#define CRUISE_INTERVAL ((uint32_t)(TIMER_FREQ_HZ_STEP / V_MAX_STEPS))

/* runtime */
typedef struct {
  const RotateBlock_t* block;
  volatile uint32_t step_index;
  volatile uint32_t current_interval;
  volatile uint32_t next_step_tick;
  volatile uint32_t tick;
} MoveExec_t;

typedef struct {
  Stepper_t* motor_rot;
  int32_t current_position;
} Rotator_t;

static MoveExec_t current_block;
static RotateBlock_t home_block;

static Rotator_t rotator = { .motor_rot = NULL, .current_position = 0 };

bool Rotator_ReturnStart(void) {
  if (Rotator_IsBusy()) {
    return false;
  }
  home_block = Rotator_GenerateBlock(-rotator.current_position);
  Rotator_StartMove(&home_block);
  return true;
}

/* PUBLIC METHODS */
void Rotator_Init(Stepper_t* rot) { rotator.motor_rot = rot; }

RotateBlock_t Rotator_GenerateBlock(int32_t steps) {
  uint32_t path_steps_uint = (uint32_t)(abs(steps));
  uint32_t accel_steps = ACCEL_STEPS_IDEAL;
  if (2 * ACCEL_STEPS_IDEAL >= path_steps_uint) {
    accel_steps = path_steps_uint / 2; /* distance to short */
  }

  interval_table_rot_t table;
  uint32_t table_len =
      accel_steps > ACCEL_STEPS_IDEAL ? ACCEL_STEPS_IDEAL : accel_steps;

  float c = TIMER_FREQ_HZ_STEP * sqrtf(2.0 / ACCEL_STEPS_S);

  for (size_t k = 0; k < table_len; k++) {
    table.interval[k] = (uint32_t)c;
    c = c - (2.0 * c) / (4.0 * k + 1);
  }

  RotateBlock_t newBlock = {
    .steps = steps,
    .accel_until = accel_steps,
    .decel_at = path_steps_uint - accel_steps,
    .cruise_interval = CRUISE_INTERVAL,
    .initial_interval = table.interval[0],
    .path_steps = path_steps_uint,
    .interval_table = table,
    .table_len = table_len,
  };

  return newBlock;
}

bool Rotator_StartMove(const RotateBlock_t* block) {
  if (Rotator_IsBusy()) {
    return false;
  }
  current_block.block = block;
  current_block.current_interval = block->initial_interval;
  current_block.next_step_tick = block->initial_interval;
  current_block.step_index = 0;
  current_block.tick = 0;

  Stepper_SetDirection(rotator.motor_rot, (current_block.block->steps > 0));

  HAL_TIM_Base_Start_IT(&htim2);
  return true;
}

void Rotator_Update(void) {
  if (current_block.block == NULL) return;

  if (rotator.motor_rot->pulse_active) {
    Stepper_ClearStep(rotator.motor_rot);
  }
  current_block.tick++;

  /* Decide with which axis to step */
  if (current_block.tick >= current_block.next_step_tick) {
    Stepper_SetStep(rotator.motor_rot);
    current_block.step_index++;

    if (current_block.step_index < current_block.block->accel_until) {
      current_block.current_interval = current_block.block->interval_table
                                           .interval[current_block.step_index];

    } else if (current_block.step_index < current_block.block->decel_at) {
      current_block.current_interval = current_block.block->cruise_interval;
    } else {
      uint32_t mirror =
          current_block.block->path_steps - current_block.step_index - 1;
      mirror = mirror > (current_block.block->table_len - 1)
                   ? current_block.block->table_len - 1
                   : mirror;

      current_block.current_interval =
          current_block.block->interval_table.interval[mirror];
    }
    current_block.next_step_tick += current_block.current_interval;
  }
  if (current_block.step_index >= current_block.block->path_steps) {
    rotator.current_position += current_block.block->steps;
    if (current_block.block == &home_block) {
      rotator.current_position = 0;
    }
    current_block.block = NULL;
    Stepper_ClearStep(rotator.motor_rot);
  }
}

bool Rotator_IsBusy(void) { return (current_block.block != NULL); }
