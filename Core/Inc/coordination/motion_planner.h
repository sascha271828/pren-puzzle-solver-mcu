#ifndef __MOTION_PLANNER_H__
#define __MOTION_PLANNER_H__

#include "step_generator.h"

// TODO: Richtige Werte einsetzten
#define STEPS_PER_MM_X (80.0f)
#define STEPS_PER_MM_Y (80.0f)

#define OFFSET_PICK_X_MM (10.0f)
#define OFFSET_PICK_Y_MM (10.0f)

#define OFFSET_PLACE_X_MM (250.0f)
#define OFFSET_PLACE_Y_MM (10.0f)

void MotionPlanner_Init(void);

MoveBlock_t MotionPlanner_PlanMoveToPickMM(float pick_x_mm, float pick_y_mm);
MoveBlock_t MotionPlanner_PlanMoveToPlaceMM(float place_x_mm, float place_y_mm);

#endif /* __MOTION_PLANNER_H__ */