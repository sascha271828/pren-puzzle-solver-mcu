#include "main.h"

#include "buttons.h"

#include "utils.h"

static GPIO_Pin_t btn_Start = {
  .port = DIN_10_GPIO_Port,
  .pin = DIN_10_Pin,
};

static bool btn_Start_Pressed = false;

void Buttons_Poll_ISR(void) {
  if (HAL_GPIO_ReadPin(btn_Start.port, btn_Start.pin) == GPIO_PIN_SET) {
    btn_Start_Pressed = true;
  }
}

bool Buttons_Start_Pressed(void) { return btn_Start_Pressed; }
void Buttons_Start_RearmPressDetection(void) { btn_Start_Pressed = false; }