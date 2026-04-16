#include "main.h"

#include "piston.h"

static Piston_t piston;
/**
 * CONFIG_PISTON_TICKS_RETRACT_INIT
 * CONFIG_PISTON_TICKS_START_MOVE
 * CONFIG_PISTON_TICKS_START_GRAB
 * CONFIG_PISTON_TICKS_START_RELEASE
 * CONFIG_PISTON_TICKS_MOVE_GRAB
 * CONFIG_PISTON_TICKS_MOVE_RELEASE
 * CONFIG_PISTON_TICKS_GRAB_RELEASE
 */

/* [current][target] */
static uint32_t state_transition[PISTON_POS_COUNT][PISTON_POS_COUNT] = {
  { 0,
    CONFIG_PISTON_TICKS_START_MOVE,
    CONFIG_PISTON_TICKS_START_GRAB,
    CONFIG_PISTON_TICKS_START_RELEASE },
  { CONFIG_PISTON_TICKS_START_MOVE,
    0,
    CONFIG_PISTON_TICKS_MOVE_GRAB,
    CONFIG_PISTON_TICKS_MOVE_RELEASE },
  { CONFIG_PISTON_TICKS_START_GRAB,
    CONFIG_PISTON_TICKS_MOVE_GRAB,
    0,
    CONFIG_PISTON_TICKS_GRAB_RELEASE },
  { CONFIG_PISTON_TICKS_START_RELEASE,
    CONFIG_PISTON_TICKS_MOVE_RELEASE,
    CONFIG_PISTON_TICKS_GRAB_RELEASE,
    0 },
};

/* ---- PRIVATE FUNCTIONS ---- */
/* GPIO / small logic Wrappers (H-Bridge) */
static void Piston_SetExtend(void) {
  piston.is_moving = true;
  HAL_GPIO_WritePin(
      piston.piston_extend.port, piston.piston_extend.pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(
      piston.piston_retract.port, piston.piston_retract.pin, GPIO_PIN_SET);
}
static void Piston_SetRetract(void) {
  piston.is_moving = true;
  HAL_GPIO_WritePin(
      piston.piston_extend.port, piston.piston_extend.pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(
      piston.piston_retract.port, piston.piston_retract.pin, GPIO_PIN_RESET);
}
static void Piston_Stop(void) {
  piston.is_moving = false;
  HAL_GPIO_WritePin(
      piston.piston_extend.port, piston.piston_extend.pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(
      piston.piston_retract.port, piston.piston_retract.pin, GPIO_PIN_SET);
}

/* ---- PUBLIC API ---- */

void Piston_Init(GPIO_Pin_t pin_extend, GPIO_Pin_t pin_retract) {
  piston.piston_extend = pin_extend;
  piston.piston_retract = pin_retract;

  piston.current = PISTON_POS_RELEASE; /* assuming worst case scenario */
  piston.ticks_until = CONFIG_PISTON_TICKS_RETRACT_INIT;
  piston.target = PISTON_POS_START;
  Piston_SetRetract();
}

/* call periodicaly */
void Piston_Update(void) {
  if (!piston.is_moving) {
    return;
  }

  piston.ticks_until--;
  if (piston.ticks_until <= 0) {
    Piston_Stop();
    piston.current = piston.target;
  }
}

void Piston_Set(PistonLogical_e target_pos) {
  if (piston.is_moving) {
    return;
  }
  if (piston.current == target_pos) {
    return;
  }
  piston.target = target_pos;
  piston.ticks_until = (int32_t)state_transition[piston.current][piston.target];
  if (((uint32_t)piston.target - (uint32_t)piston.current) > 0) {
    Piston_SetExtend();
  } else {
    Piston_SetRetract();
  }
}

void Piston_Abort(void) {
  /* TODO maybe add option to recover */
  Piston_Stop();
  piston.current = PISTON_POS_RELEASE;
  piston.target = PISTON_POS_RELEASE;
}

bool Piston_IsBusy(void) { return piston.is_moving; }