#ifndef __MOTION_PLANNER_H__
#define __MOTION_PLANNER_H__

#include "step_generator.h"

/**
 * @file motion_planner.h
 * @brief Translates physical millimeter coordinates into step generator move
 * blocks.
 *
 * The motion planner acts as the bridge between the logical coordinate system
 * (millimeters) and the physical actuators (stepper motors). It maintains the
 * current absolute position of the robot head internally to calculate the
 * necessary relative movements (deltas) for the next path seamlessly.
 *
 * It supports independent offset configurations for different physical work
 * areas (e.g., a "Pick" area for unsorted pieces and a "Place" area for the
 * completed puzzle).
 */

// TODO: Richtige Werte einsetzen
/** @brief Hardware scaling factor: X-axis stepper steps per millimeter */
#define STEPS_PER_MM_X (80.0f)
/** @brief Hardware scaling factor: Y-axis stepper steps per millimeter */
#define STEPS_PER_MM_Y (80.0f)

/** @brief X-axis distance from the physical zero (endstops) to the A4 pick area
 * origin */
#define OFFSET_PICK_X_MM (10.0f)
/** @brief Y-axis distance from the physical zero (endstops) to the A4 pick area
 * origin */
#define OFFSET_PICK_Y_MM (10.0f)

/** @brief X-axis distance from the physical zero (endstops) to the A5 place
 * area origin */
#define OFFSET_PLACE_X_MM (250.0f)
/** @brief Y-axis distance from the physical zero (endstops) to the A5 place
 * area origin */
#define OFFSET_PLACE_Y_MM (10.0f)

/**
 * @brief Initializes the motion planner.
 * Resets the internal absolute position tracker to (0, 0).
 * Must be called once before any planning functions are used.
 */
void MotionPlanner_Init(void);

/**
 * @brief Plans a linear trajectory to a coordinate within the "Pick" (A4) area.
 * Applies the pick-specific offsets and converts the target to stepper pulses.
 *
 * @param pick_x_mm  Target X coordinate relative to the pick area origin (in
 * mm).
 * @param pick_y_mm  Target Y coordinate relative to the pick area origin (in
 * mm).
 * @return MoveBlock_t  A fully initialized step profile ready for
 * StepGenerator_StartMove().
 */
MoveBlock_t MotionPlanner_PlanMoveToPickMM(float pick_x_mm, float pick_y_mm);

/**
 * @brief Plans a linear trajectory to a coordinate within the "Place" (A5)
 * area. Applies the place-specific offsets and converts the target to stepper
 * pulses.
 *
 * @param place_x_mm Target X coordinate relative to the place area origin (in
 * mm).
 * @param place_y_mm Target Y coordinate relative to the place area origin (in
 * mm).
 * @return MoveBlock_t  A fully initialized step profile ready for
 * StepGenerator_StartMove().
 */
MoveBlock_t MotionPlanner_PlanMoveToPlaceMM(float place_x_mm, float place_y_mm);

#endif /* __MOTION_PLANNER_H__ */