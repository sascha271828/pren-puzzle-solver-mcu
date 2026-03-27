#include "motion_planner.h"

static int32_t current_pos_steps_x = 0;
static int32_t current_pos_steps_y = 0;

void MotionPlanner_Init(void) {
    current_pos_steps_x = 0;
    current_pos_steps_y = 0;
}

void MotionPlanner_SetCurrentPosMM(float x_mm, float y_mm) {
    current_pos_steps_x = (int32_t)(x_mm * STEPS_PER_MM_X);
    current_pos_steps_y = (int32_t)(y_mm * STEPS_PER_MM_Y);
}

MoveBlock_t MotionPlanner_PlanMoveToMM(float target_x_mm, float target_y_mm) {
    int32_t target_steps_x = (int32_t)(target_x_mm * STEPS_PER_MM_X);
    int32_t target_steps_y = (int32_t)(target_y_mm * STEPS_PER_MM_Y);
    
    int32_t delta_x = target_steps_x - current_pos_steps_x;
    int32_t delta_y = target_steps_y - current_pos_steps_y;
    
    current_pos_steps_x = target_steps_x;
    current_pos_steps_y = target_steps_y;
    
    return StepGenerator_GenerateBlock(delta_x, delta_y);
}