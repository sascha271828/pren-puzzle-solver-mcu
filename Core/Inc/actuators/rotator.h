#ifndef __ROTATION_H__
#define __ROTATION_H__

#include "stepper.h"

typedef enum {
  ROTATOR_ERROR = 1,
  ROTATOR_IDLE,
  ROTATOR_HOMING,
} RotatorState_e;

typedef struct {
  StepperPin_t pins;
  int32_t current_position;
  RotatorState_e state;
  volatile StepperMicro_e current_micro;
  volatile bool pulse_active;
  volatile bool direction;
} Rotator_t;

/* METHODS */

void Rotator_Init(Rotator_t *self, StepperPin_t pins);
void Rotator_ReturnStart(void);

void Rotator_SetDirection(Rotator_t *self, bool dir);
void Rotator_SetMicro(StepperMicro_e micro);

void Rotator_SetStep(Rotator_t *self);

void Rotator_ClearStep(Rotator_t *self);
#if CONFIG_FOR_ENABLE_DRIVER
void Rotator_Enable(Rotator_t *self, bool enable);
#endif

void Stepper_SetMicrostep(Rotator_t *self, StepperMicro_e resolution);

#endif /* __ROTATION_H__ */