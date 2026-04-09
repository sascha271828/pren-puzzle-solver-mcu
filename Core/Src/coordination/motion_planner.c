#include "motion_planner.h"
#include "step_generator.h"

static int32_t current_pos_steps_x = 0;
static int32_t current_pos_steps_y = 0;

void MotionPlanner_Init(void) {
  current_pos_steps_x = 0;
  current_pos_steps_y = 0;
}

static MoveBlock_t PlanMoveToAbsoluteMM(float absolute_x_mm,
                                        float absolute_y_mm) {
  
  int32_t target_steps_x = (int32_t)(absolute_x_mm * CONFIG_STEPS_PER_MM_X);
  int32_t target_steps_y = (int32_t)(absolute_y_mm * CONFIG_STEPS_PER_MM_Y);

  int32_t delta_x = target_steps_x - current_pos_steps_x;
  int32_t delta_y = target_steps_y - current_pos_steps_y;

  current_pos_steps_x = target_steps_x;
  current_pos_steps_y = target_steps_y;

  return StepGenerator_GenerateBlock(delta_x, delta_y);
}

MoveBlock_t MotionPlanner_PlanMoveToPickMM(float pick_x_mm, float pick_y_mm) {
  float abs_x = pick_x_mm + CONFIG_OFFSET_PICK_X_MM;
  float abs_y = pick_y_mm + CONFIG_OFFSET_PICK_Y_MM;

  return PlanMoveToAbsoluteMM(abs_x, abs_y);
}

MoveBlock_t MotionPlanner_PlanMoveToPlaceMM(float place_x_mm,
                                            float place_y_mm) {
  float abs_x = place_x_mm + CONFIG_OFFSET_PLACE_X_MM;
  float abs_y = place_y_mm + CONFIG_OFFSET_PLACE_Y_MM;

  return PlanMoveToAbsoluteMM(abs_x, abs_y);
}