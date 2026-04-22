#include "main.h"

#include "piston.h"

static Piston_t piston;
static uint32_t pwm_count;
static volatile bool pwm_extending;
static volatile bool pwm_change;

static const uint32_t pos_offsets_ms[PISTON_POS_COUNT] = {
  PISTON_MS_TO_TICKS(PISTON_OFFSET_START_MS),
  PISTON_MS_TO_TICKS(PISTON_OFFSET_MOVE_MS),
  PISTON_MS_TO_TICKS(PISTON_OFFSET_GRAB_MS),
  PISTON_MS_TO_TICKS(PISTON_OFFSET_RELEASE_MS)
};

/* ---- PRIVATE FUNCTIONS ---- */
/* GPIO / small logic Wrappers (H-Bridge) */
static void Piston_SetExtend(void) {
  pwm_extending = true;
  piston.is_moving = true;
  HAL_GPIO_WritePin(
      piston.piston_extend.port, piston.piston_extend.pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(
      piston.piston_retract.port, piston.piston_retract.pin, GPIO_PIN_SET);
}
static void Piston_SetRetract(void) {
  pwm_extending = false;
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
  piston.ticks_until = PISTON_MS_TO_TICKS(CONFIG_PISTON_TICKS_RETRACT_INIT);
  piston.target = PISTON_POS_START;
  Piston_SetRetract();
}

/* call periodicaly */
void Piston_Update(void) {
  if (!piston.is_moving) {
    return;
  }
  pwm_count--;
  /* off */
  if (pwm_count <= CONFIG_PISTON_PWM_ENUMERATER) {
    if (pwm_change) {
      if (pwm_extending) {
        HAL_GPIO_WritePin(
            piston.piston_extend.port, piston.piston_extend.pin, GPIO_PIN_SET);
      } else {
        HAL_GPIO_WritePin(piston.piston_retract.port,
                          piston.piston_retract.pin,
                          GPIO_PIN_SET);
      }
    }
    /* on */
    if (pwm_count == 0) {
      pwm_count = CONFIG_PISTON_PWM_ENUMERATER;
      if (pwm_extending) {
        HAL_GPIO_WritePin(piston.piston_extend.port,
                          piston.piston_extend.pin,
                          GPIO_PIN_RESET);

      } else {
        HAL_GPIO_WritePin(piston.piston_retract.port,
                          piston.piston_retract.pin,
                          GPIO_PIN_RESET);
      }
    }
  } else {
    pwm_change = true;
    piston.ticks_until--;
    if (piston.ticks_until <= 0) {
      Piston_Stop();
      piston.current = piston.target;
    }
  }
}

void Piston_Set(PistonLogical_e target_pos) {
  if (piston.is_moving || piston.current == target_pos) return;

  // Berechnung der Zeit-Differenz (immer positiv durch abs oder check)
  uint32_t duration_ms;
  if (target_pos > piston.current) {
    duration_ms = pos_offsets_ms[target_pos] - pos_offsets_ms[piston.current];
    Piston_SetExtend();
  } else {
    duration_ms = pos_offsets_ms[piston.current] - pos_offsets_ms[target_pos];
    Piston_SetRetract();
  }

  piston.target = target_pos;
  piston.ticks_until = duration_ms;
}

void Piston_Abort(void) {
  /* TODO maybe add option to recover */
  Piston_Stop();
  piston.current = PISTON_POS_RELEASE;
  piston.target = PISTON_POS_RELEASE;
}

bool Piston_IsBusy(void) { return piston.is_moving; }