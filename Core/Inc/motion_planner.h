#ifndef __MOTION_PLANNER_H__
#define __MOTION_PLANNER_H__

#include "step_generator.h"


typedef struct{
    Stepper_t *motor_x;
    Stepper_t *motor_y;
    

    /* machine constants */
    float steps_per_mm_x;
    float steps_per_mm_y;

    /* limits */
    float max_velocity;      // mm/s
    float max_acceleration;  // mm/s^2


    /* optinal */
    volatile float current_x_mm;    
    volatile float current_y_mm;
}MotionPlanner_t;

void Planner_Init(MotionPlanner_t *self, Stepper_t *mx, Stepper_t *my);
void Planner_SetStepsPerMM(MotionPlanner_t *self, float steps_x, float steps_y);
void Planner_SetLimits(MotionPlanner_t *self, float max_vel, float max_accel);


// Blocking move (waits until move completes)
void Planner_MoveTo(MotionPlanner_t *self, float x_mm, float y_mm, float feedrate_mm_per_s);

// Non‑blocking start
bool Planner_StartMoveTo(MotionPlanner_t *self, float x_mm, float y_mm, float feedrate_mm_per_s);
bool Planner_IsBusy(MotionPlanner_t *self);
void Planner_WaitForIdle(MotionPlanner_t *self);
void Planner_Abort(MotionPlanner_t *self);

#endif