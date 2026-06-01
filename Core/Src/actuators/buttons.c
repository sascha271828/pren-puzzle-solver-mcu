#include "main.h"

#include "buttons.h"

#include "utils.h"

/* ========================
 *   PRIVATE VARIABLES
 * ======================== */

static GPIO_Pin_t btn_Start = {
  .port = DIN_10_GPIO_Port,
  .pin = DIN_10_Pin,
};

static GPIO_Pin_t btn_Reset = {
  .port = DIN_11_GPIO_Port,
  .pin = DIN_11_Pin,
};

static volatile bool btn_Start_Pressed = false;
static volatile bool btn_Reset_Pressed = false;

/* ========================
 *   PUBLIC API
 * ======================== */

void Buttons_Poll_ISR(void) {
  if (HAL_GPIO_ReadPin(btn_Start.port, btn_Start.pin) == GPIO_PIN_RESET) {
    btn_Start_Pressed = true;
  }
  if (HAL_GPIO_ReadPin(btn_Reset.port, btn_Reset.pin) == GPIO_PIN_RESET) {
    btn_Reset_Pressed = true;
  }
}

bool Buttons_Start_Pressed(void) { return btn_Start_Pressed; }

void Buttons_Start_RearmPressDetection(void) { btn_Start_Pressed = false; }

bool Buttons_Reset_Pressed(void) { return btn_Reset_Pressed; }

void Buttons_Reset_RearmPressDetection(void) { btn_Reset_Pressed = false; }
