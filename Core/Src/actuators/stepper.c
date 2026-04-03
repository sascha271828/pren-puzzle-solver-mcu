#include "main.h"

#include "stepper.h"

static uint32_t Stepper_ResolutionConversion(StepperMicro_e res) {
  switch (res) {
    case STEP_FULL:
      return 0b00;
    case STEP_1_2:
      return 0b10;
    case STEP_1_4:
      return 0b11;
    case STEP_1_16:
      return 0b01;
    default:
      return 0b11;
  }
}

void Stepper_Init(Stepper_t* self,
                  StepperPin_t pins,
                  StepperMicro_e micro,
                  bool enable) {
  self->pins = pins;
  /* TODO implement homing */
  self->direction = false;
  self->is_homed = false;
  self->pulse_active = false;
  self->state = STEPPER_IDLE;
  self->current_position = 0;

  Stepper_SetMicrostep(self, micro);
}

void Stepper_SetDirection(Stepper_t* self, bool dir) {
  HAL_GPIO_WritePin(self->pins.dir.port,
                    self->pins.dir.pin,
                    dir ? GPIO_PIN_SET : GPIO_PIN_RESET);
  self->direction = dir;
}

void Stepper_SetStep(Stepper_t* self) {
  HAL_GPIO_WritePin(self->pins.step.port, self->pins.step.pin, GPIO_PIN_SET);

  self->pulse_active = true;
  self->current_position += (self->direction ? 1 : -1);
}

void Stepper_ClearStep(Stepper_t* self) {
  HAL_GPIO_WritePin(self->pins.step.port, self->pins.step.pin, GPIO_PIN_RESET);

  self->pulse_active = false;
}

void Stepper_Enable(Stepper_t* self, bool enable) {
  HAL_GPIO_WritePin(self->pins.enable.port,
                    self->pins.enable.pin,
                    enable ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void Stepper_SetMicrostep(Stepper_t* self, StepperMicro_e res) {
  uint32_t micro = Stepper_ResolutionConversion(res);
  HAL_GPIO_WritePin(self->pins.m0.port,
                    self->pins.m0.pin,
                    micro & (1 << 0) ? GPIO_PIN_SET : GPIO_PIN_RESET);

  HAL_GPIO_WritePin(self->pins.m1.port,
                    self->pins.m1.pin,
                    micro & (1 << 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);

  self->current_micro = res;
}
