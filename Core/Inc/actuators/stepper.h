#ifndef __STEPPER_H__
#define __STEPPER_H__

#include "sys_config.h"
#include "utils.h"

#include <stdbool.h>

#define STEPPER_MICRO_FULL (0)
#define STEPPER_MICRO_1_16 (1)
#define STEPPER_MICRO_1_2 (2)
#define STEPPER_MICRO_1_4 (3)

/* can be used for microsteps bitwise M1 M0 */
typedef enum {
  STEP_FULL = STEPPER_MICRO_FULL,
  STEP_1_16 = STEPPER_MICRO_1_16,
  STEP_1_2 = STEPPER_MICRO_1_2,
  STEP_1_4 = STEPPER_MICRO_1_4
} StepperMiro_e;

typedef enum {
  STEPPER_ERROR = 1,
  STEPPER_IDLE,
  STEPPER_HOMING,
  STEPPER_MOVING
} StepperState_e;

typedef struct {
#if CONFIG_FOR_ENABLE_DRIVER
  GPIO_Pin_t enable;
#endif
#if CONFIG_FOR_NSLEEP_DRIVER
  GPIO_Pin_t nsleep;
#endif
#if CONFIG_STEPPER_NFAULT
  GPIO_Pin_t fault;
#endif
  GPIO_Pin_t step;
  GPIO_Pin_t dir;
  GPIO_Pin_t m0;
  GPIO_Pin_t m1;
  GPIO_Pin_t limit_switch_min;
  GPIO_Pin_t limit_switch_max;
} StepperPin_t;

/*  STEPPER STRUCT  */
typedef struct {
  StepperPin_t pins;

  volatile StepperState_e state;
  volatile int32_t current_position;
#if CONFIG_STEPPER_MICRO
  volatile StepperMiro_e current_micro;
#endif
#if CONFIG_FOR_ENABLE_DRIVER
  volatile bool is_enabled;
#endif

  volatile bool direction;
  volatile bool is_homed;
  volatile bool pulse_active;
#if CONFIG_STEPPER_NFAULT
  volatile bool has_fault;
#endif
} Stepper_t;

/*  METHODS */
void Stepper_Init(Stepper_t *self, StepperPin_t pins);
void Stepper_SetDirection(Stepper_t *self, bool dir);
void Stepper_SetStep(Stepper_t *self);
void Stepper_ClearStep(Stepper_t *self);
#if CONFIG_FOR_ENABLE_DRIVER
void Stepper_Enable(Stepper_t *self, bool enable);
#endif
void Stepper_SetMicrostep(Stepper_t *self, StepperMiro_e resolution);

#endif /* __STEPPER_H__ */