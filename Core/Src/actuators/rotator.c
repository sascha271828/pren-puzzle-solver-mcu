#include "main.h"

#include "rotator.h"

#include <assert.h>

void Rotator_Init(Rotator_t *self, StepperPin_t pins) { assert(false); }

void Rotator_ReturnStart(void) {
  assert(false);
} /* to avoid cables getting caught */

void Rotator_SetDirection(Rotator_t *self, bool dir) {
  HAL_GPIO_WritePin(self->pins.dir.port,
                    self->pins.dir.pin,
                    dir ? GPIO_PIN_SET : GPIO_PIN_RESET);
  self->direction = dir;
}
void Rotator_SetMicro(StepperMicro_e micro);

void Rotator_SetStep(Rotator_t *self) {
  HAL_GPIO_WritePin(self->pins.step.port, self->pins.step.pin, GPIO_PIN_SET);

  self->pulse_active = true;
  /* TODO update according to micro steps */
  assert(false);
  self->current_position += (self->direction ? 1 : -1);
}

void Rotator_ClearStep(Rotator_t *self) {
  HAL_GPIO_WritePin(self->pins.step.port, self->pins.step.pin, GPIO_PIN_RESET);

  self->pulse_active = false;
}

#if CONFIG_FOR_ENABLE_DRIVER
void Rotator_Enable(Rotator_t *self, bool enable) {
  HAL_GPIO_WritePin(self->pins->enable->port,
                    self->pins->enable->pin,
                    enable ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
#endif

void Stepper_SetMicrostep(Rotator_t *self, StepperMicro_e resolution) {
  HAL_GPIO_WritePin(self->pins.m1.port,
                    self->pins.m1.pin,
                    resolution & (1 << 0) ? GPIO_PIN_SET : GPIO_PIN_RESET);

  HAL_GPIO_WritePin(self->pins.m1.port,
                    self->pins.m1.pin,
                    resolution & (1 << 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);

  self->current_micro = resolution;
}
