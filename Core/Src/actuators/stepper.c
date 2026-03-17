#include "main.h"

#include "stepper.h"

void Stepper_Init(Stepper_t *self, StepperPin_t pins) {
  self->pins = pins;
  /* TODO implement */
  self->direction = false;
  self->is_homed = false;
  self->pulse_active = false;
  self->state = STEPPER_IDLE;
  self->current_position = 0;
}

void Stepper_SetDirection(Stepper_t *self, bool dir) {
  HAL_GPIO_WritePin(self->pins.dir.port,
                    self->pins.dir.pin,
                    dir ? GPIO_PIN_SET : GPIO_PIN_RESET);
  self->direction = dir;
}

void Stepper_SetStep(Stepper_t *self) {
  HAL_GPIO_WritePin(self->pins.step.port, self->pins.step.pin, GPIO_PIN_SET);

  self->pulse_active = true;
  self->current_position += (self->direction ? 1 : -1);
}

void Stepper_ClearStep(Stepper_t *self) {
  HAL_GPIO_WritePin(self->pins.step.port, self->pins.step.pin, GPIO_PIN_RESET);

  self->pulse_active = false;
}

#if CONFIG_FOR_ENABLE_DRIVER
void Stepper_Enable(Stepper_t *self, bool enable) {
  HAL_GPIO_WritePin(self->pins->enable->port,
                    self->pins->enable->pin,
                    enable ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
#endif

void Stepper_SetMicrostep(Stepper_t *self, StepperMiro_e resolution) {
  HAL_GPIO_WritePin(self->pins.m1.port,
                    self->pins.m1.pin,
                    resolution & (1 << 0) ? GPIO_PIN_SET : GPIO_PIN_RESET);

  HAL_GPIO_WritePin(self->pins.m1.port,
                    self->pins.m1.pin,
                    resolution & (1 << 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);

  self->current_position = resolution;
}
