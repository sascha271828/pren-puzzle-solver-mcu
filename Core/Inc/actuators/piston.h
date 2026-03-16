#ifndef __PISTON_H__
#define __PISTON_H__

#include "sys_config.h"
#include "utils.h"

#include <stdbool.h>

typedef enum {
  PISTON_INIT, /* only at the beginning */
  PISTON_START_POS,
  PISTON_EXTENDING,
  PISTON_RETRACTING,
  PISTON_GRAB,
  PISTON_MOVE_POS,
  PISTON_RELEASE
} PistonState_e;

typedef struct {
  volatile PistonState_e state;

  const GPIO_Pin_t piston_1_extend;
  const GPIO_Pin_t piston_1_retract;

#if CONFIG_PISTON_SEPARAT_PINS
  const GPIO_Pin_t piston_2_extend;
  const GPIO_Pin_t piston_2_retract;
#endif

#if CONFIG_PISTON_HAS_LIMIT_SWITCH
  GPIO_Pin_t limit_switch_extended;
  GPIO_Pin_t limit_switch_contracted;
#endif

} Piston_t;

bool Piston_Set(Piston_t* self, PistonState_e action);
void Piston_Stop(Piston_t* self);

#endif /* __PISTON_H__ */