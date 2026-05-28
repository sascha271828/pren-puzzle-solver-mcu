#include "main.h"

#include "magnet.h"

/* the state needs to be reversed because a separate switch is used */

/* ========================
 *   PRIVATE VARIABLES
 * ======================== */

static GPIO_Pin_t MagnetPin;
static bool MagnetActive = false;
static uint32_t CountDelay = 0;
static uint32_t PwmCount = CONFIG_MAGNET_PWM_DIVISOR;

/* ========================
 *   PUBLIC API
 * ======================== */

void Magnet_Init(GPIO_Pin_t pin) { MagnetPin = pin; }

void Magnet_SetState(bool state) {
  if (state) {
    CountDelay = 0;
    MagnetPwm = CONFIG_MAGNET_PWM_DIVISOR;
    MagnetActive = true;
    HAL_GPIO_WritePin(MagnetPin.port, MagnetPin.pin, GPIO_PIN_RESET);
  } else {
    MagnetActive = false;
    HAL_GPIO_WritePin(MagnetPin.port, MagnetPin.pin, GPIO_PIN_SET);
  }
}

void Magnet_Process(void) {
  if (!MagnetActive) {
    HAL_GPIO_WritePin(MagnetPin.port, MagnetPin.pin, GPIO_PIN_SET);
  } else {
    CountDelay++;
    if (CountDelay >= CONFIG_MAGNET_DELAY_TICKS) {
      PwmCount--;

      if (PwmCount <= CONFIG_PISTON_PWM_ENUMERATER) {
        HAL_GPIO_WritePin(MagnetPin.port, MagnetPin.pin, GPIO_PIN_RESET);
      } else {
        HAL_GPIO_WritePin(MagnetPin.port, MagnetPin.pin, GPIO_PIN_SET);
      }

      if (PwmCount == 0) {
        PwmCount = CONFIG_PISTON_PWM_DIVISOR;
      }

    }   
  }
  if (CountDelay >= CONFIG_MAGNET_TIMEOUT_TICKS){
    Magnet_SetState(false);
  } 
}

bool Magnet_GetState(void) { return MagnetActive; }