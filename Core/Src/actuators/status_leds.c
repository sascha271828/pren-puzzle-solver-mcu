#include "main.h"

#include "status_leds.h"

#include <stdbool.h>

/* clang-format off */
static GPIO_Pin_t status_leds[STATUSLED_SENTINEL] = { 
    { 
        .port = DOUT_5_GPIO_Port,
        .pin = DOUT_5_Pin
    }, 
    { 
        .port = DOUT_6_GPIO_Port,
        .pin = DOUT_6_Pin
    }, 
    { 
        .port = DOUT_7_GPIO_Port,
        .pin = DOUT_7_Pin
    }
};
/* clang-format on */

static bool led_blinking_mode[STATUSLED_SENTINEL] = { false, false, false };
static bool led_blinking_on = false;
static uint32_t led_blinking_tick = 0;

static void StatusLeds_Off_All(void) {
  HAL_GPIO_WritePin(status_leds[STATUSLED_GREEN].port,
                    status_leds[STATUSLED_GREEN].pin,
                    GPIO_PIN_RESET);
  HAL_GPIO_WritePin(status_leds[STATUSLED_YELLOW].port,
                    status_leds[STATUSLED_YELLOW].pin,
                    GPIO_PIN_RESET);
  HAL_GPIO_WritePin(status_leds[STATUSLED_RED].port,
                    status_leds[STATUSLED_RED].pin,
                    GPIO_PIN_RESET);
}

void StatusLeds_Init(void) { StatusLeds_Off_All(); }

StatusLeds_Type_e StatusLed_Read(StatusLeds_e led) {
  if (led_blinking_mode[led] == true) {
    return STATUSLED_TYPE_BLINK;
  }
  return (HAL_GPIO_ReadPin(status_leds[led].port, status_leds[led].pin) ==
          GPIO_PIN_SET)
             ? STATUSLED_TYPE_ON
             : STATUSLED_TYPE_OFF;
}

void StatusLeds_On(StatusLeds_e led) {
  led_blinking_mode[led] = false;
  HAL_GPIO_WritePin(status_leds[led].port, status_leds[led].pin, GPIO_PIN_SET);
}

void StatusLeds_Off(StatusLeds_e led) {
  led_blinking_mode[led] = false;
  HAL_GPIO_WritePin(
      status_leds[led].port, status_leds[led].pin, GPIO_PIN_RESET);
}

void StatusLeds_Blink(StatusLeds_e led) {
  led_blinking_mode[led] = true;

  HAL_GPIO_WritePin(status_leds[led].port, status_leds[led].pin, GPIO_PIN_SET);
}

void StatusLeds_Blink_ISR(void) {
  if (led_blinking_tick == 0) {
    led_blinking_tick = STATUSLED_BLINK_TICKS;
    if (led_blinking_mode[STATUSLED_GREEN]) {
      HAL_GPIO_WritePin(status_leds[STATUSLED_GREEN].port,
                        status_leds[STATUSLED_GREEN].pin,
                        led_blinking_on ? GPIO_PIN_RESET : GPIO_PIN_SET);
    }
    if (led_blinking_mode[STATUSLED_YELLOW]) {
      HAL_GPIO_WritePin(status_leds[STATUSLED_YELLOW].port,
                        status_leds[STATUSLED_YELLOW].pin,
                        led_blinking_on ? GPIO_PIN_RESET : GPIO_PIN_SET);
    }
    if (led_blinking_mode[STATUSLED_RED]) {
      HAL_GPIO_WritePin(status_leds[STATUSLED_RED].port,
                        status_leds[STATUSLED_RED].pin,
                        led_blinking_on ? GPIO_PIN_RESET : GPIO_PIN_SET);
    }

    led_blinking_on = !led_blinking_on;
  }
  led_blinking_tick--;
}
