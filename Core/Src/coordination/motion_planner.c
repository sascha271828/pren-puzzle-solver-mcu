#include "main.h"

#include "motion_planner.h"

#include "step_generator.h"
#include "sys_config.h"

#include <math.h>
#include <stdlib.h>


static MoveBlock_t block; /* temporary solution till queue implemented */

void Planner_Init(MotionPlanner_t *self, Stepper_t *mx, Stepper_t *my) {
  self->motor_x = mx;
  self->motor_y = my;
  self->steps_per_mm_x = 1.0f;  // default, must be set later
  self->steps_per_mm_y = 1.0f;
  self->current_x_mm = 0.0f;
  self->current_y_mm = 0.0f;
  self->max_velocity = 0.0f;
  self->max_acceleration = 0.0f;
}

void Planner_SetStepsPerMM(MotionPlanner_t *self,
                           float steps_x,
                           float steps_y) {
  self->steps_per_mm_x = steps_x;
  self->steps_per_mm_y = steps_y;
}

void Planner_SetLimits(MotionPlanner_t *self, float max_vel, float max_accel) {
  self->max_velocity = max_vel;
  self->max_acceleration = max_accel;
}

static bool create_move_block(MotionPlanner_t *self,
                              float target_x_mm,
                              float target_y_mm,
                              float feedrate_mm_s,
                              MoveBlock_t *block) {
  // Compute step differences
  int32_t dx = (int32_t)roundf((target_x_mm - self->current_x_mm) *
                               self->steps_per_mm_x);
  int32_t dy = (int32_t)roundf((target_y_mm - self->current_y_mm) *
                               self->steps_per_mm_y);

  if (dx == 0 && dy == 0) return false;  // no move

  block->steps_x = dx;
  block->steps_y = dy;

  // Set direction in stepper structs (simplified – you might want a function
  // Stepper_SetDirection) Here we assume the stepper struct has a 'direction'
  // field that will be used by Stepper_SetStep. The step generator will rely on
  // that field.
  if (dx >= 0)
    self->motor_x->direction = 1;  // forward
  else
    self->motor_x->direction = 0;  // reverse
  // Similarly for Y
  if (dy >= 0)
    self->motor_y->direction = 1;
  else
    self->motor_y->direction = 0;

  // For barebones, we ignore acceleration – just constant speed.
  // Compute total path steps (longest axis)
  uint32_t abs_x = abs(dx);
  uint32_t abs_y = abs(dy);
  uint32_t path_steps = (abs_x > abs_y) ? abs_x : abs_y;
  if (path_steps == 0) path_steps = 1;  // prevent division by zero

  block->path_steps = path_steps;

  // Convert feedrate to step interval
  // feedrate in mm/s -> steps/s = feedrate * steps_per_mm (use the axis with
  // more steps? simpler: use path length)
  float path_mm = sqrtf(
      (target_x_mm - self->current_x_mm) * (target_x_mm - self->current_x_mm) +
      (target_y_mm - self->current_y_mm) * (target_y_mm - self->current_y_mm));
  float steps_per_sec = feedrate_mm_s * path_steps / path_mm;  // approximate

  // Timer ticks per step (assuming 100 kHz tick)
  uint32_t interval_ticks = (uint32_t)(100000.0f / steps_per_sec);
  if (interval_ticks < 2) interval_ticks = 2;  // minimum

  block->cruise_interval = interval_ticks;
  block->initial_interval = interval_ticks;  // no acceleration, so same
  block->accel_until = 0;                    // no accel phase
  block->decel_at = path_steps;  // decel immediately? Actually with constant
                                 // speed, no decel phase needed.

  // Reset runtime fields (step generator will initialise these)
  block->counter_x = 0;
  block->counter_y = 0;
  block->step_index = 0;
  block->current_interval = interval_ticks;
  block->next_step_tick = 0;  // will be set by step generator when block starts

  return true;
}

bool Planner_StartMoveTo(MotionPlanner_t *self,
                         float x_mm,
                         float y_mm,
                         float feedrate_mm_s) {
  if (!create_move_block(self, x_mm, y_mm, feedrate_mm_s, &block))
    return false;  // no move needed

  // Add block to step generator (you'll need a queue; for barebones, just
  // overwrite current)
  bool ok = StepGenerator_AddBlock(&block);
  if (ok) {
    // Update current position immediately (or wait until move completes)
    self->current_x_mm = x_mm;
    self->current_y_mm = y_mm;
  }
  return ok;
}

void Planner_MoveTo(MotionPlanner_t *self,
                    float x_mm,
                    float y_mm,
                    float feedrate_mm_s) {
  if (Planner_StartMoveTo(self, x_mm, y_mm, feedrate_mm_s)) {
    Planner_WaitForIdle(self);
  }
}

bool Planner_IsBusy(MotionPlanner_t *self) { return StepGenerator_IsBusy(); }

void Planner_WaitForIdle(MotionPlanner_t *self) {
  while (Planner_IsBusy(self)) {
    // optional: low-power wait, or just loop
  }
}

void Planner_Abort(MotionPlanner_t *self) { StepGenerator_Abort(); }