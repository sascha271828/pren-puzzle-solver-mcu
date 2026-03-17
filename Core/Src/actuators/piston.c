#include "main.h"

#include "piston.h"

#define PISTON_TIMEOUT_MS 2000  // safety fallback if no limit switch

// ── internal helpers ────────────────────────────────────────────────────────

void set_extend_pins(Piston_t* self, GPIO_PinState state) {
  HAL_GPIO_WritePin(
      self->piston_1_extend.port, self->piston_1_extend.pin, state);
#if CONFIG_PISTON_SEPARAT_PINS
  HAL_GPIO_WritePin(
      self->piston_2_extend.port, self->piston_2_extend.pin, state);
#endif
}

void set_retract_pins(Piston_t* self, GPIO_PinState state) {
  HAL_GPIO_WritePin(
      self->piston_1_retract.port, self->piston_1_retract.pin, state);
#if CONFIG_PISTON_SEPARAT_PINS
  HAL_GPIO_WritePin(
      self->piston_2_retract.port, self->piston_2_retract.pin, state);
#endif
}

static void begin_move(Piston_t* self,
                       PistonPhysical_e direction,
                       uint32_t timeout_ms) {
  // Cut power to opposite direction first (never energise both)

  set_extend_pins(self, GPIO_PIN_RESET);
  set_retract_pins(self, GPIO_PIN_RESET);

  if (direction == PISTON_STATE_EXTENDED) {
    set_extend_pins(self, GPIO_PIN_SET);
  } else {
    set_retract_pins(self, GPIO_PIN_SET);
  }
  self->target = direction;
  self->move_deadline_tick = HAL_GetTick() + timeout_ms;
  self->physical = PISTON_STATE_MOVING;
}

// ── public API ───────────────────────────────────────────────────────────────

void Piston_Init(Piston_t* self) {
  set_extend_pins(self, GPIO_PIN_RESET);
  set_retract_pins(self, GPIO_PIN_RESET);
  self->physical = PISTON_STATE_UNKNOWN;
  self->target = PISTON_STATE_UNKNOWN;
}

// Call from your timer ISR or main-loop tick.
// Returns true when motion is complete (or timed out).
PistonResult_e Piston_Update(Piston_t* self) {
  if (self->physical != PISTON_STATE_MOVING) return PISTON_OK;

#if CONFIG_PISTON_HAS_LIMIT_SWITCH
  GPIO_Pin_t* sw = (self->target == PISTON_STATE_EXTENDED)
                       ? &self->limit_switch_extended
                       : &self->limit_switch_retracted;

  if (HAL_GPIO_ReadPin(sw->port, sw->pin) == GPIO_PIN_SET) {
    Piston_Stop(self);  // limit switch hit → done
    return PISTON_OK;
  }
#endif

  // Timeout fallback (works with or without limit switches)
  if (HAL_GetTick() >= self->move_deadline_tick) {
    Piston_Stop(self);
    return PISTON_OK;
  }

  return PISTON_BUSY;
}

void Piston_Stop(Piston_t* self) {
  set_extend_pins(self, GPIO_PIN_RESET);
  set_retract_pins(self, GPIO_PIN_RESET);
  self->physical = self->target;  // best knowledge of where we are
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
      out->direction = PISTON_STATE_RETRACTED;
      out->timeout_ms = CONFIG_PISTON_TIME_RETRACT_INIT;
      return true;

    case PISTON_POS_GRAB:
      if (from != PISTON_POS_MOVE) return false;
      out->direction = PISTON_STATE_EXTENDED;
      out->timeout_ms = CONFIG_PISTON_TIME_GRAB_MOVE_MS;
      return true;

    case PISTON_POS_MOVE:
      switch (from) {
        case PISTON_POS_START:
          out->direction = PISTON_STATE_EXTENDED;
          out->timeout_ms = CONFIG_PISTON_TIME_START_MOVE_MS;
          return true;
        case PISTON_POS_GRAB:
          out->direction = PISTON_STATE_RETRACTED;
          out->timeout_ms = CONFIG_PISTON_TIME_GRAB_MOVE_MS;
          return true;
        case PISTON_POS_RELEASE:
          out->direction = PISTON_STATE_RETRACTED;
          out->timeout_ms = CONFIG_PISTON_TIME_MOVE_RELEASE_MS;
          return true;
        default:
          return false; /* illegal transition */
      }

    case PISTON_POS_RELEASE:
      if (from != PISTON_POS_MOVE) return false;
      out->direction = PISTON_STATE_EXTENDED;
      out->timeout_ms = CONFIG_PISTON_TIME_MOVE_RELEASE_MS;
      return true;

    default:
      return false;
  }
}

PistonResult_e Piston_Set(Piston_t* self, PistonLogical_e target_pos) {
  if (self->physical == PISTON_STATE_MOVING) return PISTON_ERR_BUSY;
  if (self->logical == target_pos) return PISTON_OK;

  MoveParams_t params;
  if (!resolve_move(self->logical, target_pos, &params)) {
    return PISTON_ERR_INVALID_TRANSITION;
  }

  begin_move(self, params.direction, params.timeout_ms);

  // ── blocking wait (replace with state-machine loop if you go async) ──
  PistonResult_e result;
  while ((result = Piston_Update(self)) == PISTON_BUSY) {
    ;  // or __WFI() to sleep until next interrupt
  }

  if (result == PISTON_OK) {
    self->logical = target_pos;
  }
  return result;
}