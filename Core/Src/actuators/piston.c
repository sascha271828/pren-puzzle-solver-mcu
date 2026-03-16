#include "main.h"

#include "piston.h"

extern volatile bool piston_moving;

static void Piston_Extend(Piston_t* self, uint32_t time) {
  if (self->state == PISTON_RETRACTING) {
    Piston_Stop(self);
  }
  Start_Piston_Timer(time);
  HAL_GPIO_WritePin(
      self->piston_1_retract.port, self->piston_1_retract.pin, GPIO_PIN_SET);
#if CONFIG_PISTON_SEPARAT_PINS
  HAL_GPIO_WritePin(
      self->piston_2_retract.port, self->piston_2_retract.pin, GPIO_PIN_SET);
  piston_moving = true;
  self->state = PISTON_EXTENDING;
#endif
}

static void Piston_Retract(Piston_t* self, uint32_t time) {
  if (self->state == PISTON_EXTENDING) {
    Piston_Stop(self);
  }
  Start_Piston_Timer(time);
  HAL_GPIO_WritePin(
      self->piston_1_extend.port, self->piston_1_extend.pin, GPIO_PIN_SET);
#if CONFIG_PISTON_SEPARAT_PINS
  HAL_GPIO_WritePin(
      self->piston_2_extend.port, self->piston_2_extend.pin, GPIO_PIN_SET);
  piston_moving = true;
  self->state = PISTON_RETRACTING;
#endif
}

/* TODO: Optimize retraction time */

void Piston_Stop(Piston_t* self) {
  if (self->state == PISTON_EXTENDING) {
    HAL_GPIO_WritePin(
        self->piston_1_extend.port, self->piston_1_extend.pin, GPIO_PIN_RESET);
#if CONFIG_PISTON_SEPARAT_PINS
    HAL_GPIO_WritePin(
        self->piston_2_extend.port, self->piston_2_extend.pin, GPIO_PIN_RESET);
#endif
  } else if (self->state == PISTON_RETRACTING) {
    HAL_GPIO_WritePin(self->piston_1_retract.port,
                      self->piston_1_retract.pin,
                      GPIO_PIN_RESET);
#if CONFIG_PISTON_SEPARAT_PINS
    HAL_GPIO_WritePin(self->piston_2_retract.port,
                      self->piston_2_retract.pin,
                      GPIO_PIN_RESET);
#endif
  }
}

bool Piston_Set(Piston_t* self, PistonState_e action) {
  if (self->state == action) return true;

  switch (action) {
    case PISTON_START_POS:
      Piston_Retract(self, CONFIG_PISTON_TIME_RETRACT_INIT);
      break;
    case PISTON_GRAB:
      Piston_Extend(self, CONFIG_PISTON_TIME_GRAB_MOVE_MS);
      break;
    case PISTON_MOVE_POS:
      switch (self->state) {
        case PISTON_START_POS:
          Piston_Extend(self, CONFIG_PISTON_TIME_START_MOVE_MS);
          break;
        case PISTON_GRAB:
          Piston_Retract(self, CONFIG_PISTON_TIME_GRAB_MOVE_MS);
          break;
        case PISTON_RELEASE:
          Piston_Retract(self, CONFIG_PISTON_TIME_MOVE_RELEASE_MS);
          break;
        default:
          return false;
      }
      break;
    case PISTON_RELEASE:
      Piston_Extend(self, CONFIG_PISTON_TIME_MOVE_RELEASE_MS);
      break;
    default:
      return false;
  }
  while (piston_moving == true) {
    ; /* TODO: currently doing nothing */
  }
  Piston_Stop(self);
  self->state = action;
  return true;
}
