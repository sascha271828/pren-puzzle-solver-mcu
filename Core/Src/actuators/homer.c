#include "homer.h"

#include "step_generator.h"

static MoveBlock_t home_block;

bool homing = false;

void Homer_HomingStart(void) {
  home_block =
      StepGenerator_GenerateBlock(-HOMER_HOMING_STEPS, -HOMER_HOMING_STEPS);
  homing = true;
  StepGenerator_StartMove(&home_block);
}
