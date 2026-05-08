#include "main.h"

#include "emergency_stop.h"

#include "buttons.h"
#include "interrupt.h"
#include "status_leds.h"

static GPIO_Pin_t emergency_stop = {
  .port = DIN_9_GPIO_Port,
  .pin = DIN_9_Pin,
};

void EmergencyStop_Process(void) {
  if (HAL_GPIO_ReadPin(emergency_stop.port, emergency_stop.pin) ==
      GPIO_PIN_SET) {
    Interrupt_SetState(IS_ESTOP);
    Buttons_Reset_RearmPressDetection();
    StatusLeds_Blink(STATUSLED_RED);
    StatusLeds_Off(STATUSLED_YELLOW);
    StatusLeds_Off(STATUSLED_GREEN);
  }
}

bool EmergencyStop_IsActivated(void) {
  if (HAL_GPIO_ReadPin(emergency_stop.port, emergency_stop.pin) ==
      GPIO_PIN_SET) {
    return true;
  }
  return false;
}
