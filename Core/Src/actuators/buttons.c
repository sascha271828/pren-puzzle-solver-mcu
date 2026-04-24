#include "main.h"

#include "buttons.h"

#include "utils.h"

static GPIO_Pin_t btn_Start = {
  .port = DIN_10_GPIO_Port,
  .pin = DIN_10_Pin,
};

static GPIO_Pin_t btn_Reset = {
  .port = DIN_12_GPIO_Port,
  .pin = DIN_12_Pin,
};

static bool btn_Start_Pressed = false;
static bool btn_Reset_Pressed = false;

void Buttons_Poll_ISR(void) {
  if (HAL_GPIO_ReadPin(btn_Start.port, btn_Start.pin) == GPIO_PIN_SET) {
    btn_Start_Pressed = true;
  }
  if (HAL_GPIO_ReadPin(btn_Reset.port, btn_Reset.pin) == GPIO_PIN_SET) {
    btn_Reset_Pressed = true;
  }
}

bool Buttons_Start_Pressed(void) { return btn_Start_Pressed; }
void Buttons_Start_RearmPressDetection(void) { btn_Start_Pressed = false; }

bool Buttons_Reset_Pressed(void) { return btn_Reset_Pressed; }

void Buttons_Reset_RearmPressDetection(void) { btn_Reset_Pressed = false; }
