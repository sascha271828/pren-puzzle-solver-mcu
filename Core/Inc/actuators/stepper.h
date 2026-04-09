#ifndef __STEPPER_H__
#define __STEPPER_H__

#include "sys_config.h"
#include "utils.h"

#include <stdbool.h>

/* can be used for microsteps bitwise M1 M0 */
typedef enum {
  STEP_FULL = STEPPER_MICRO_FULL,
  STEP_1_2 = STEPPER_MICRO_1_2,
  STEP_1_4 = STEPPER_MICRO_1_4,
  STEP_1_16 = STEPPER_MICRO_1_16,
} StepperMicro_e;

typedef enum {
  STEPPER_ERROR = 1,
  STEPPER_IDLE,
  STEPPER_HOMING,
  STEPPER_MOVING
} StepperState_e;

typedef struct {
  GPIO_Pin_t enable;
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
  volatile StepperMicro_e current_micro;
  volatile bool is_enabled;
  volatile bool direction;
  volatile bool is_homed;
  volatile bool pulse_active;
#if CONFIG_STEPPER_NFAULT
  volatile bool has_fault;
#endif
} Stepper_t;

/*  METHODS */
void Stepper_Init(Stepper_t* self,
                  StepperPin_t pins,
                  StepperMicro_e micro,
                  bool enable);
void Stepper_SetDirection(Stepper_t* self, bool dir);
void Stepper_SetStep(Stepper_t* self);
void Stepper_ClearStep(Stepper_t* self);
void Stepper_SetMicrostep(Stepper_t* self, StepperMicro_e res);
void Stepper_Enable(Stepper_t* self, bool enable);
#endif /* __STEPPER_H__ */