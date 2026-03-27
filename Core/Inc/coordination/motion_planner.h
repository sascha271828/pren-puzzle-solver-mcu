#ifndef __MOTION_PLANNER_H__
#define __MOTION_PLANNER_H__

#include "step_generator.h"
#include "sys_config.h"

// TODO:  Werte an Mechanik anpassen!
#define STEPS_PER_MM_X (80.0f)
#define STEPS_PER_MM_Y (80.0f)

void MotionPlanner_Init(void);
void MotionPlanner_SetCurrentPosMM(float x_mm, float y_mm);

MoveBlock_t MotionPlanner_PlanMoveToMM(float target_x_mm, float target_y_mm);

#endif /* __MOTION_PLANNER_H__ */