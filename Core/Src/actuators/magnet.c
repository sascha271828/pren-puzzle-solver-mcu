#include "main.h"

#include "magnet.h"

static GPIO_Pin_t* MagnetPin;

void Magnet_Init(GPIO_Pin_t* pin) { MagnetPin = pin; }

void Magnet_Grab(bool state) {
  HAL_GPIO_WritePin(
      MagnetPin->port, MagnetPin->pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}