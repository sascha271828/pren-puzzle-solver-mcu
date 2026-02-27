#ifndef __MOTION_PLANER_H__
#define __MOTION_PLANER_H__

#include "stepper.h"


typedef struct{
    Stepper_t *motor_x;
    Stepper_t *motor_y;
    

    /* machine constants */
    float steps_per_mm_x;
    float steps_per_mm_y;

    /* limits */
    float max_velocity;      // mm/s
    float max_acceleration;  // mm/s^2

    bool is_busy;
    float current_x_mm;
    float current_y_mm;
}MotionPlaner_t;



void Planner_Init(MotionPlaner_t* self, Stepper_t *motor_x, Stepper_t* motor_y);

void Planer_SetMachineConstants(MotionPlaner_t* self,
    float steps_per_mm_x,   float steps_per_mm_y
    );

void Planner_SetLimit(MotionPlaner_t* self,
    float max_velocity,     float max_acceleration
    );


void Planner_MoveTo(MotionPlaner_t* self, float x, float y);

bool Planner_IsBusy(MotionPlaner_t* self);

static void _Planner_CalculateSync(MotionPlaner_t* self, int32_t dx, int32_t dy);

void Planner_Abort(void);


#endif