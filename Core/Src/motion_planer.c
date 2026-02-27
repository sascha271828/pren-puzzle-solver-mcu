#include "motion_planer.h"




void Planner_Init(MotionPlaner_t* self, Stepper_t *motor_x, Stepper_t* motor_y){
    self->motor_x = motor_x;
    self->motor_y = motor_y;
    self->current_x_mm = 0;
    self->current_y_mm = 0;
    self->is_busy = false;
}




void Planer_SetMachineConstants(MotionPlaner_t* self, float steps_per_mm_x, float steps_per_mm_y){
    self->steps_per_mm_x = steps_per_mm_x;
    self->steps_per_mm_y = steps_per_mm_y;
}

void Planner_SetLimit(MotionPlaner_t* self, float max_velocity, float max_acceleration){
    self->max_velocity = max_velocity;
    self->max_acceleration = max_acceleration;
}