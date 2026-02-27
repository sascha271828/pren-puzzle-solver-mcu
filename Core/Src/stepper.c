#include "stepper.h"


void Stepper_Init(Stepper_t *self, StepperPin_t pins){
    self->pins = pins;

    self->state = STEPPER_IDLE;
    self->current_position = 0;

    self->target_position = 0;

    self->target_steps = 0;
    self->accel_until = 0;
    self->decel_at = 0;

    self->current_interval = 0;
    self->min_interval = 0;
    self->max_interval = 0;

    self->acceleration_rate = 0;
    self->interval_accumulator = 0;

    self->pulse_start_time = 0;

    self->microstop_resolution = 0;


    #if CONFIG_FOR_ENABLE_DRIVER
    self->is_enabled = false;
    #endif

    self->direction = false;
    self->is_homed = false;
    self->is_inverted = false;

}




