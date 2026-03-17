#include "main.h"

#include "step_generator.h"

#include "sys_config.h"
#include "tim.h"

#include <stdbool.h>
#include <stdlib.h>

static MoveBlock_t *current_block = NULL;
/* temporary, will need to copy the data later on */

static Stepper_t *motor_x = NULL;
static Stepper_t *motor_y = NULL;

static bool x_dominant = false;

void StepGenerator_Init(Stepper_t *mx, Stepper_t *my) {
  motor_x = mx;
  motor_y = my;
}

MoveBlock_t StepGenerator_GenerateBlock(int32_t steps_x,
                                        uint32_t steps_y,
                                        uint32_t accel_until,
                                        uint32_t decel_at,
                                        uint32_t cruise_interval,
                                        uint32_t intial_interval) {
  bool x_is_dominant = (abs(steps_x)) > abs(steps_y);
  uint32_t internal_path_steps = x_is_dominant ? abs(steps_x) : abs(steps_y);
  MoveBlock_t newBlock = { .steps_x = steps_x,
                           .steps_y = steps_y,
                           .accel_until = accel_until,
                           .decel_at = decel_at,
                           .cruise_interval = cruise_interval,
                           .initial_interval = intial_interval,

                           .counter = (uint32_t)(internal_path_steps / 2),
                           .step_index = 0,
                           .path_steps = internal_path_steps,
                           .current_interval = 0,
                           .next_step_tick = 0,
                           .x_dominant = x_is_dominant,
                           .steps_minor =
                               !x_is_dominant ? abs(steps_x) : abs(steps_y) };
  return newBlock;
}

void StepGenerator_StartStep(MoveBlock_t *block) {
  current_block = block;
  system_tick = 0;
  HAL_TIM_Base_Start_IT(&htim2);
}

void StepGenerator_Update(void) {
  if (current_block == NULL) return;

  if (motor_x->pulse_active) {
    Stepper_ClearStep(motor_x);
  }
  if (motor_y->pulse_active) {
    Stepper_ClearStep(motor_y);
  }

  /* Decide with which axis to step */
  if (system_tick >= current_block->next_step_tick) {
    current_block->counter += current_block->steps_minor;

    /* step */
    if (x_dominant) {
      Stepper_SetStep(motor_x);
      if (current_block->counter >= current_block->path_steps) {
        Stepper_SetStep(motor_y);
        current_block->counter -= current_block->path_steps;
      }
    } else {
      Stepper_SetStep(motor_y);
      if (current_block->counter >= current_block->path_steps) {
        Stepper_SetStep(motor_x);
        current_block->counter -= current_block->path_steps;
      }
    }

    current_block->step_index++;

    if (current_block->step_index < current_block->accel_until) {
      // Decrease interval each step (accelerate)
      // e.g. Bresenham-style: interval -= (2 * interval) / (4 * step_index +
      // 1);
      ; /* TODO: */
      current_block->current_interval = current_block->initial_interval;
    } else if (current_block->step_index < current_block->decel_at) {
      current_block->current_interval = current_block->cruise_interval;
    } else {
      // Increase interval each step (decelerate)
      current_block->current_interval = current_block->cruise_interval;
    }
    current_block->next_step_tick += current_block->current_interval;
  }
  if (current_block->step_index >= current_block->path_steps) {
    current_block = NULL;
    HAL_TIM_Base_Stop_IT(&htim2);
    Stepper_ClearStep(motor_x);
    Stepper_ClearStep(motor_y);
  }
}

bool StepGenerator_IsBusy(void) { return (current_block != NULL); }