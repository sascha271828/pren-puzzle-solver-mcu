#ifndef __MOTION_PLANNER_H__
#define __MOTION_PLANNER_H__

#include "step_generator.h"
#include "sys_config.h"

/**
 * @file motion_planner.h
 * @brief Translates physical millimeter coordinates into step generator move
 * blocks.
 */

/* --- Hardware Scaling Factors (Derived from sys_config.h) --- */

/** @brief Steps per millimeter for X/Y axis */
#define CONFIG_STEPS_PER_MM_X ((float)AXIS_STEPS_PER_MM_NUM / (float)AXIS_STEPS_PER_MM_DEN)
#define CONFIG_STEPS_PER_MM_Y ((float)AXIS_STEPS_PER_MM_NUM / (float)AXIS_STEPS_PER_MM_DEN)

/** @brief Steps per 0.1 degree for rotation */
#define CONFIG_STEPS_PER_01_DEGREE (((float)CONFIG_ROT_STEPS_PER_REV * (float)CONFIG_ROT_MICRO) / 3600.0f)

/* --- Work Area Offsets (Must be measured on physical hardware) --- */

/** @brief Distance from endstops to the pick area origin [mm]. */
#define CONFIG_OFFSET_PICK_X_MM (-5.2f)
#define CONFIG_OFFSET_PICK_Y_MM (126.0f)

/**
 * @brief Linear scale correction applied to incoming pick coordinates before
 *        motion planning. Compensates for mechanical stretch / calibration
 *        error of the pick area. Applied by the caller (state_machine.c)
 *        before passing coordinates to MotionPlanner_PlanMoveToPickMM().
 */
#define CONFIG_CORRECTION_PICK_X (0.9975f)  //(0.995f)
#define CONFIG_CORRECTION_PICK_Y (0.9945f)  //(0.995f)

/** @brief Distance from endstops to the place area origin [mm]. */
#define CONFIG_OFFSET_PLACE_X_MM (48.5f)
#define CONFIG_OFFSET_PLACE_Y_MM (-12.0f)

/**
 * @brief Initialises the motion planner and resets the internal absolute
 *        position tracker to (0, 0).  Must be called once after homing.
 */
void MotionPlanner_Init(void);

/**
 * @brief Computes a relative MoveBlock_t from the current tracked position
 *        to the pick area coordinate (pick_x_mm + CONFIG_OFFSET_PICK_X_MM,
 *        pick_y_mm + CONFIG_OFFSET_PICK_Y_MM).  Updates the internal
 *        position tracker.
 *
 * @param pick_x_mm  Pick-area X coordinate relative to the pick origin [mm].
 * @param pick_y_mm  Pick-area Y coordinate relative to the pick origin [mm].
 * @return MoveBlock_t  Ready-to-execute move block for StepGenerator_StartMove().
 */
MoveBlock_t MotionPlanner_PlanMoveToPickMM(float pick_x_mm, float pick_y_mm);

/**
 * @brief Computes a relative MoveBlock_t from the current tracked position
 *        to the place area coordinate (place_x_mm + CONFIG_OFFSET_PLACE_X_MM,
 *        place_y_mm + CONFIG_OFFSET_PLACE_Y_MM).  Updates the internal
 *        position tracker.
 *
 * @param place_x_mm  Place-area X coordinate relative to the place origin [mm].
 * @param place_y_mm  Place-area Y coordinate relative to the place origin [mm].
 * @return MoveBlock_t  Ready-to-execute move block for StepGenerator_StartMove().
 */
MoveBlock_t MotionPlanner_PlanMoveToPlaceMM(float place_x_mm, float place_y_mm);

#endif /* __MOTION_PLANNER_H__ */