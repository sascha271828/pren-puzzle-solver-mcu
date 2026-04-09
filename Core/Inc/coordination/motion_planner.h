#ifndef __MOTION_PLANNER_H__
#define __MOTION_PLANNER_H__

#include "step_generator.h"
#include "sys_config.h"

/**
 * @file motion_planner.h
 * @brief Translates physical millimeter coordinates into step generator move blocks.
 */

/* --- Hardware Scaling Factors (Derived from sys_config.h) --- */

/** @brief Steps per millimeter for X/Y axis */
#define CONFIG_STEPS_PER_MM_X ((float)AXIS_STEPS_PER_MM_NUM / (float)AXIS_STEPS_PER_MM_DEN)
#define CONFIG_STEPS_PER_MM_Y ((float)AXIS_STEPS_PER_MM_NUM / (float)AXIS_STEPS_PER_MM_DEN)

/** @brief Steps per 0.1 degree for rotation */
#define CONFIG_STEPS_PER_01_DEGREE ((float)ROT_STEPS_PER_MM_NUM / (float)ROT_STEPS_PER_MM_DEN)

/* --- Work Area Offsets (Must be measured on physical hardware) --- */

/** @brief Distance from endstops to the pick area origin */
#define CONFIG_OFFSET_PICK_X_MM (10.0f)
#define CONFIG_OFFSET_PICK_Y_MM (10.0f)

/** @brief Distance from endstops to the place area origin */
#define CONFIG_OFFSET_PLACE_X_MM (250.0f)
#define CONFIG_OFFSET_PLACE_Y_MM (10.0f)

/**
 * @brief Initializes the motion planner.
 * Resets the internal absolute position tracker to (0, 0).
 */
void MotionPlanner_Init(void);

/**
 * @brief Plans a trajectory to a coordinate within the "Pick" area.
 */
MoveBlock_t MotionPlanner_PlanMoveToPickMM(float pick_x_mm, float pick_y_mm);

/**
 * @brief Plans a trajectory to a coordinate within the "Place" area.
 */
MoveBlock_t MotionPlanner_PlanMoveToPlaceMM(float place_x_mm, float place_y_mm);

#endif /* __MOTION_PLANNER_H__ */