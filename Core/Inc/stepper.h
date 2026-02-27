#ifndef __STEPPER_H__
#define __STEPPER_H__

#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"

#include "utils.h" 


#define CONFIG_FOR_NFAULT_DRIVER    (1)     /* wheter the nFAULT detection of the stepper drivers is implemented */

#define CONFIG_FOR_ENABLE_DRIVER    (1)     /* wheter the ENABLE pins for the stepper drivers are controlled through software */
#define CONFIG_FOR_NSLEEP_DRIVER    (1)     /* wheter the nSLEEP pins for the stepper drivers are controlled through software */


/* TODO: get set microstep implementation done*/

typedef enum{
    STEPPER_IDLE,
    STEPPER_MOVING,
    STEPPER_HOMING,
    STEPPER_ERROR
}StepperState_t;


typedef struct{
#if CONFIG_FOR_ENABLE_DRIVER
    GPIO_Pin_t enable; 
#endif
#if CONFIG_FOR_NSLEEP_DRIVER
    GPIO_Pin_t nsleep; 
#endif
#if CONFIG_FOR_NFAULT_DRIVER
    GPIO_Pin_t fault; 
#endif
    GPIO_Pin_t step; 
    GPIO_Pin_t dir; 
    GPIO_Pin_t m0; 
    GPIO_Pin_t m1; 
}StepperPin_t;


/*  STEPPER STRUCT  */
typedef struct{
    StepperPin_t pin;
    volatile StepperState_t state;
    volatile int32_t current_position;

    /* motion */
    int32_t target_position;
    
    /* non linear */
    uint32_t target_steps;
    uint32_t accel_until;
    uint32_t decel_at;

    uint32_t current_interval;
    uint32_t min_interval;
    uint32_t max_interval;
    
    float acceleartion_rate;
    float interval_accumulator;

    uint32_t pulse_start_time;    
    
    /* states */
    uint8_t microstop_resolution;

    #if CONFIG_FOR_ENABLE_DRIVER
    bool is_enabled;
    #endif
    bool direction;
    bool is_homed;
    bool is_inverted;       /* to flip directions from software in needed */
    volatile bool has_fault;
}Stepper_t;


/*  METHODS */
void Stepper_Init(Stepper_t *self, StepperPin_t pins);

/* prepare */
void Stepper_SetTarget(Stepper_t *self, int32_t target_steps);
void Stepper_PrepareMove(Stepper_t *self, int32_t relative_steps, uint32_t speed, uint32_t accel);


void Stepper_SetMicrostep(Stepper_t *self);
void Stepper_Update(Stepper_t *self);

bool Stepper_IsBusy(Stepper_t *self);



/*
{
    self->total_steps = abs(relative_steps);
    self->target_position = self->current_position + relative_steps;
    
    // Logic to calculate ramp points
    // For a simple trapezoid, accel_until is usually 1/3 of total_steps 
    // unless the move is too short to reach top speed.
    self->accel_until = self->total_steps / 3; 
    self->decel_at = self->total_steps - self->acc_until;
    
    self->current_interval = self->max_interval; // Start slow
    self->state = STEPPER_MOVING;
}*/


#endif