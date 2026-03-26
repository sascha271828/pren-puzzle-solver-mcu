#include "main.h"

#include "piston.h"

#define PISTON_TIMEOUT_MS 2000  // safety fallback if no limit switch

static Piston_t piston;

static void set_extend_pins(GPIO_PinState state) {
  HAL_GPIO_WritePin(
      piston.piston_1_extend.port, piston.piston_1_extend.pin, state);
#if CONFIG_PISTON_SEPARAT_PINS
  HAL_GPIO_WritePin(
      piston.piston_2_extend.port, piston.piston_2_extend.pin, state);
#endif
}

static void set_retract_pins(GPIO_PinState state) {
  HAL_GPIO_WritePin(
      piston.piston_1_retract.port, piston.piston_1_retract.pin, state);
#if CONFIG_PISTON_SEPARAT_PINS
  HAL_GPIO_WritePin(
      pisotn.piston_2_retract.port, piston.piston_2_retract.pin, state);
#endif
}

static void Piston_Stop(void) {
  set_extend_pins(GPIO_PIN_RESET);
  set_retract_pins(GPIO_PIN_RESET);
}

static void begin_move(PistonPhysical_e direction, uint32_t timeout_ms) {
  // Cut power to opposite direction first (never energise both)

  set_extend_pins(GPIO_PIN_RESET);
  set_retract_pins(GPIO_PIN_RESET);

  if (direction == PISTON_STATE_EXTENDING) {
    set_extend_pins(GPIO_PIN_SET);
  } else {
    set_retract_pins(GPIO_PIN_SET);
  }
  piston.target = direction;
  piston.move_deadline_tick = HAL_GetTick() + timeout_ms;
  piston.physical = PISTON_STATE_MOVING;
}

// ── public API ───────────────────────────────────────────────────────────────

void Piston_Init(GPIO_Pin_t pin_extend, GPIO_Pin_t pin_retract) {
  piston.piston_1_extend = pin_extend;
  piston.piston_1_retract = pin_retract;
  piston.physical = PISTON_STATE_UNKNOWN;
  piston.target = PISTON_STATE_UNKNOWN;
  piston.logical = PISTON_POS_UNKNOWN;
  piston.target_logical = PISTON_STATE_UNKNOWN;
  set_extend_pins(GPIO_PIN_RESET);
  set_retract_pins(GPIO_PIN_RESET);
}

// Call from your timer ISR or main-loop tick.
// Returns true when motion is complete (or timed out).
void Piston_Update(void) {
  if (piston.physical == PISTON_STATE_SET ||
      piston.physical == PISTON_STATE_UNKNOWN) {
    return;
  }

#if CONFIG_PISTON_HAS_LIMIT_SWITCH
  GPIO_Pin_t* sw = (piston.target == PISTON_STATE_EXTENDING)
                       ? &piston.limit_switch_extended
                       : &piston.limit_switch_retracted;

  if (HAL_GPIO_ReadPin(piston.port, piston.pin) == GPIO_PIN_SET) {
    Piston_Stop();  // limit switch hit → done
  }
#endif

  // Timeout fallback (works with or without limit switches)
  /* TODO change to own ticks not HAL_GetTick*/
  if (HAL_GetTick() >= piston.move_deadline_tick) {
    Piston_Stop();
    piston.logical = piston.target_logical;
    piston.target_logical = PISTON_STATE_UNKNOWN;
    piston.target = PISTON_STATE_SET;
  }
}

// Map logical positions to physical direction + timing
typedef struct {
  PistonPhysical_e direction;
  uint32_t timeout_ms;
} MoveParams_t;

static bool resolve_move(PistonLogical_e from,
                         PistonLogical_e to,
                         MoveParams_t* out) {
  // Encode (from → to) transitions explicitly.
  // Return false for illegal transitions.
  switch (to) {
    case PISTON_POS_START:
      out->direction = PISTON_STATE_RETRACTING;
      out->timeout_ms = CONFIG_PISTON_TIME_RETRACT_INIT;
      return true;

    case PISTON_POS_GRAB:
      if (from != PISTON_POS_MOVE) return false;
      out->direction = PISTON_STATE_EXTENDING;
      out->timeout_ms = CONFIG_PISTON_TIME_GRAB_MOVE_MS;
      return true;

    case PISTON_POS_MOVE:
      switch (from) {
        case PISTON_POS_START:
          out->direction = PISTON_STATE_EXTENDING;
          out->timeout_ms = CONFIG_PISTON_TIME_START_MOVE_MS;
          return true;
        case PISTON_POS_GRAB:
          out->direction = PISTON_STATE_RETRACTING;
          out->timeout_ms = CONFIG_PISTON_TIME_GRAB_MOVE_MS;
          return true;
        case PISTON_POS_RELEASE:
          out->direction = PISTON_STATE_RETRACTING;
          out->timeout_ms = CONFIG_PISTON_TIME_MOVE_RELEASE_MS;
          return true;
        default:
          return false; /* illegal transition */
      }

    case PISTON_POS_RELEASE:
      if (from != PISTON_POS_MOVE) return false;
      out->direction = PISTON_STATE_EXTENDING;
      out->timeout_ms = CONFIG_PISTON_TIME_MOVE_RELEASE_MS;
      return true;

    default:
      return false;
  }
}

PistonResult_e Piston_Set(PistonLogical_e target_pos) {
  if (piston.physical == PISTON_STATE_MOVING) return PISTON_ERR_BUSY;
  if (piston.logical == target_pos) return PISTON_OK;

  MoveParams_t params;
  if (!resolve_move(piston.logical, target_pos, &params)) {
    return PISTON_ERR_INVALID_TRANSITION;
  }
  piston.target_logical = target_pos;

  begin_move(params.direction, params.timeout_ms);

  return PISTON_BUSY;
}

void PisotnAbort(void) {
  set_extend_pins(GPIO_PIN_RESET);
  set_retract_pins(GPIO_PIN_RESET);
  piston.physical = PISTON_STATE_UNKNOWN;
}