#include "main.h"

#include "step_generator.h"

#include "sys_config.h"
#include "tim.h"

#include <stdbool.h>
#include <stdlib.h>

#define V_MAX_STEPS \
  (CONFIG_MAX_SPEED_AXIS * CONFIG_STEPS_PER_MM_X) /* steps/s */
#define ACCEL_STEPS_S \
  (CONFIG_ACCEL_AXIS_MM_S2 * CONFIG_STEPS_PER_MM_X) /* steps/s */
#define ACCEL_STEPS_IDEAL (V_MAX_STEPS * V_MAX_STEPS) / (2 * ACCEL_STEPS_S)
#define CRUISE_INTERVAL ((uint32_t)(TIMER_FREQ_HZ_STEP / V_MAX_STEPS))

/* runtime */
typedef struct {
  const MoveBlock_t* block;
  volatile int32_t dda_counter;
  volatile uint32_t step_index;
  volatile uint32_t current_interval;
  volatile uint32_t next_step_tick;
  uint32_t steps_minor;
} MoveExec_t;

static MoveExec_t current_block;

static Stepper_t* motor_x = NULL;
static Stepper_t* motor_y = NULL;

void StepGenerator_Init(Stepper_t* mx, Stepper_t* my) {
  motor_x = mx;
  motor_y = my;
}

MoveBlock_t StepGenerator_GenerateBlock(int32_t steps_x, int32_t steps_y) {
  bool x_dom = (abs(steps_x)) > abs(steps_y);
  uint32_t path_steps_int = x_dom ? abs(steps_x) : abs(steps_y);

  uint32_t accel_steps = ACCEL_STEPS_IDEAL;

  /* trapezoid or triangle profile */
  if (2 * ACCEL_STEPS_IDEAL >= path_steps_int) {
    accel_steps = path_steps_int / 2; /* distance to short */
  }

  interval_table_t table;
  uint32_t table_len =
      accel_steps > ACCEL_STEPS_IDEAL ? ACCEL_STEPS_IDEAL : accel_steps;

  float c = TIMER_FREQ_HZ_STEP * sqrtf(2.0 / ACCEL_STEPS_S);

  for (size_t k = 0; k < table_len; k++) {
    table.interval[k] = (uint32_t)c;
    c = c - (2.0 * c) / (4.0 * k + 1);
  }

  MoveBlock_t newBlock = { .steps_x = steps_x,
                           .steps_y = steps_y,
                           .accel_until = accel_steps,
                           .decel_at = path_steps_int - accel_steps,
                           .cruise_interval = CRUISE_INTERVAL,
                           .initial_interval = table.interval[0],
                           .path_steps = path_steps_int,
                           .interval_table = table,
                           .table_len = table_len,
                           .x_dominant = x_dom };
  return newBlock;
}

void StepGenerator_StartMove(const MoveBlock_t* block) {
  current_block.block = block;
  current_block.current_interval = block->initial_interval;
  current_block.dda_counter = 0;
  current_block.next_step_tick = block->initial_interval;
  current_block.step_index = 0;
  current_block.steps_minor =
      block->x_dominant ? abs(block->steps_x) : abs(block->steps_y);
  system_tick = 0;
  HAL_TIM_Base_Start_IT(&htim2);
}

void StepGenerator_Update(void) {
  if (current_block.block == NULL) return;

  if (motor_x->pulse_active) {
    Stepper_ClearStep(motor_x);
  }
  if (motor_y->pulse_active) {
    Stepper_ClearStep(motor_y);
  }

  /* Decide with which axis to step */
  if (system_tick >= current_block.next_step_tick) {
    current_block.dda_counter += current_block.steps_minor;

    /* step */
    if (current_block.block->x_dominant) {
      Stepper_SetStep(motor_x);
      if (current_block.dda_counter >= current_block.block->path_steps) {
        Stepper_SetStep(motor_y);
        current_block.dda_counter -= current_block.block->path_steps;
      }
    } else {
      Stepper_SetStep(motor_y);
      if (current_block.dda_counter >= current_block.block->path_steps) {
        Stepper_SetStep(motor_x);
        current_block.dda_counter -= current_block.block->path_steps;
      }
    }

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
    current_block.block = NULL;
    HAL_TIM_Base_Stop_IT(&htim2);
    Stepper_ClearStep(motor_x);
    Stepper_ClearStep(motor_y);
  }
}

bool StepGenerator_IsBusy(void) { return (current_block.block != NULL); }
