#include "main.h"

#include "magnet.h"

static GPIO_Pin_t MagnetPin;

void Magnet_Init(GPIO_Pin_t pin) { MagnetPin = pin; }

void Magnet_SetState(bool state) {
  HAL_GPIO_WritePin(
      MagnetPin.port, MagnetPin.pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

bool Magnet_GetState(void) {
  return HAL_GPIO_ReadPin(MagnetPin.port, MagnetPin.pin) == GPIO_PIN_SET;
}