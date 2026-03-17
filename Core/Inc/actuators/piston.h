#ifndef __PISTON_H__
#define __PISTON_H__

#include "sys_config.h"
#include "utils.h"

#include <stdbool.h>

typedef enum {
  PISTON_STATE_UNKNOWN = 0,  // after init before first move
  PISTON_STATE_EXTENDED,
  PISTON_STATE_RETRACTED,
  PISTON_STATE_MOVING,  // transitioning, direction known via target
} PistonPhysical_e;

typedef enum {
  PISTON_POS_START,
  PISTON_POS_GRAB,
  PISTON_POS_MOVE,
  PISTON_POS_RELEASE,
} PistonLogical_e;

typedef struct {
  // Physical state
  volatile PistonPhysical_e physical;
  volatile PistonLogical_e logical;  // last successfully reached logical pos

  // Movement target for ISR/timeout context
  volatile PistonPhysical_e target;
  uint32_t move_deadline_tick;  // for timeout detection

  // Pins
  GPIO_Pin_t piston_1_extend;
  GPIO_Pin_t piston_1_retract;

#if CONFIG_PISTON_SEPARAT_PINS
  GPIO_Pin_t piston_2_extend;
  GPIO_Pin_t piston_2_retract;
#endif

#if CONFIG_PISTON_HAS_LIMIT_SWITCH
  GPIO_Pin_t limit_switch_extended;
  GPIO_Pin_t limit_switch_retracted;
#endif
} Piston_t;

typedef enum {
  PISTON_OK,
  PISTON_BUSY,
  PISTON_ERR_BUSY,
  PISTON_ERR_TIMEOUT,
  PISTON_ERR_INVALID_TRANSITION,
} PistonResult_e;

void Piston_Init(Piston_t* self);

PistonResult_e Piston_Update(Piston_t* self);
PistonResult_e Piston_Set(Piston_t* self, PistonLogical_e target_pos);
void Piston_Stop(Piston_t* self);
void set_extend_pins(Piston_t* self, GPIO_PinState state);
void set_retract_pins(Piston_t* self, GPIO_PinState state);

#endif /* __PISTON_H__ */