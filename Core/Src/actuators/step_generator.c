#include "main.h"

#include "step_generator.h"

#include "sys_config.h"

#include <stdbool.h>
#include <stdlib.h>

static MoveBlock_t *current_block = NULL;
/* temporary, will need to copy the data later on */

static Stepper_t *motor_x = NULL;
static Stepper_t *motor_y = NULL;

void StepGenerator_Init(Stepper_t *mx, Stepper_t *my) {
  motor_x = mx;
  motor_y = my;
}

bool StepGenerator_AddBlock(MoveBlock_t *block) {
  if (block == NULL) {
    return false;
  }
  current_block = block;
  uint32_t abs_x = abs(current_block->steps_x);
  uint32_t abs_y = abs(current_block->steps_y);
  if (abs_x > abs_y) {
    current_block->path_steps = abs_x;
  } else {
    current_block->path_steps = abs_y;
  }
  return true;
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
    bool x_step = false;
    bool y_step = false;

    /* Update DDA */
    current_block->counter_y += abs(current_block->steps_y);
    current_block->counter_x += abs(current_block->steps_x);

    /* decide which axis to step */
    if (current_block->counter_x >= current_block->path_steps) {
      x_step = true;
      current_block->counter_x -= current_block->path_steps;
    }

    if (current_block->counter_y >= current_block->path_steps) {
      y_step = true;
      current_block->counter_y -= current_block->path_steps;
    }

    /* step */
    if (x_step) Stepper_SetStep(motor_x);
    if (y_step) Stepper_SetStep(motor_y);

    current_block->step_index++;

    if (current_block->step_index < current_block->accel_until) {
      current_block->current_interval = current_block->initial_interval;
      ; /* TODO: */
    } else if (current_block->step_index >= current_block->accel_until) {
      ; /* TODO: */
    } else {
      current_block->current_interval = current_block->cruise_interval;
    }
    current_block->next_step_tick += current_block->current_interval;
  }
  if (current_block->step_index >= current_block->path_steps) {
    current_block = NULL;
  }
}

bool StepGenerator_IsBusy(void) { return (current_block != NULL); }