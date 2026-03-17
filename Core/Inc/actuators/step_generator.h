#ifndef __STEP_GENERATOR_H__
#define __STEP_GENERATOR_H__

#include "stepper.h"
#include "sys_config.h"

typedef struct {
  /* Move parameters (total steps) */
  int32_t steps_x;
  int32_t steps_y;

  /* Acceleration profile */
  uint32_t accel_until;
  uint32_t decel_at;
  uint32_t cruise_interval; /* minimum step interval (at full speed) in ticks */
  uint32_t initial_interval; /* starting interval (c0) in ticks */

  /* DDA accumulators (runtime) */
  int32_t counter;
  uint32_t step_index;  // path steps completed
  uint32_t path_steps;  // MAX of abs(steps_x) and abs(steps_y)
  uint32_t current_interval;
  uint32_t next_step_tick;
  uint32_t steps_minor;
  bool x_dominant;
} MoveBlock_t;

void StepGenerator_Init(Stepper_t *mx, Stepper_t *my);

MoveBlock_t StepGenerator_GenerateBlock(int32_t steps_x,
                                        uint32_t steps_y,
                                        uint32_t accel_until,
                                        uint32_t decel_at,
                                        uint32_t cruise_interval,
                                        uint32_t intial_interval);

void StepGenerator_Update(void); /* in ISR */
void StepGenerator_StartStep(MoveBlock_t *block);
bool StepGenerator_IsBusy(
    void); /* Return true if a block is currently being executed or there are
              blocks waiting in the queue.*/
void StepGenerator_Abort(
    void); /* Immediately stop all motion and clear the queue.*/

#endif /* __STEP_GENERATOR_H__ */