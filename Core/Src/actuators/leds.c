#include "main.h"

#include "leds.h"

static GPIO_Pin_t led;

void Leds_Init(GPIO_Pin_t pin) {
  led = pin;
  Leds_Set(true);
}
void Leds_Set(bool state) {
  HAL_GPIO_WritePin(led.port, led.pin, state ? GPIO_PIN_RESET : GPIO_PIN_SET);
}
bool Leds_Get(void) {
  return (HAL_GPIO_ReadPin(led.port, led.pin) ==
          GPIO_PIN_RESET); /* supposed to be reversed */
}
