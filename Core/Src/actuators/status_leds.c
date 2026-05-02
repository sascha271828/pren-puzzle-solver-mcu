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

static bool led_blinking_mode = false;
static bool led_blinking_on = false;
static uint32_t led_blinking_tick = 0;
static StatusLeds_e led_blinking_index = STATUSLED_GREEN;

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
/*
static void StatusLeds_On_All(void) {
  HAL_GPIO_WritePin(status_leds[STATUSLED_GREEN].port,
                    status_leds[STATUSLED_GREEN].pin,
                    GPIO_PIN_SET);
  HAL_GPIO_WritePin(status_leds[STATUSLED_YELLOW].port,
                    status_leds[STATUSLED_YELLOW].pin,
                    GPIO_PIN_SET);
  HAL_GPIO_WritePin(status_leds[STATUSLED_RED].port,
                    status_leds[STATUSLED_RED].pin,
                    GPIO_PIN_SET);
}
*/

void StatusLeds_Init(void) { StatusLeds_Off_All(); }
/*
static void StatusLeds_On_Ind(StatusLeds_e led) {
  HAL_GPIO_WritePin(status_leds[led].port, status_leds[led].pin, GPIO_PIN_SET);
}
*/

void StatusLeds_On(StatusLeds_e led) {
  HAL_NVIC_DisableIRQ(TIM2_IRQn);
  led_blinking_mode = false;
  HAL_NVIC_EnableIRQ(TIM2_IRQn);

  StatusLeds_Off_All();
  HAL_GPIO_WritePin(status_leds[led].port, status_leds[led].pin, GPIO_PIN_SET);
}

void StatusLeds_Blink(StatusLeds_e led) {
  StatusLeds_Off_All();

  HAL_NVIC_DisableIRQ(TIM2_IRQn);
  led_blinking_mode = true;
  led_blinking_index = led;
  HAL_NVIC_EnableIRQ(TIM2_IRQn);

  HAL_GPIO_WritePin(status_leds[led].port, status_leds[led].pin, GPIO_PIN_SET);
}

void StatusLeds_Blink_ISR(void) {
  if (!led_blinking_mode) {
    return;
  }
  if (led_blinking_tick == 0) {
    if (led_blinking_on) {
      HAL_GPIO_WritePin(status_leds[led_blinking_index].port,
                        status_leds[led_blinking_index].pin,
                        GPIO_PIN_RESET);
    } else {
      HAL_GPIO_WritePin(status_leds[led_blinking_index].port,
                        status_leds[led_blinking_index].pin,
                        GPIO_PIN_SET);
    }
    led_blinking_on = !led_blinking_on;
    led_blinking_tick = STATUSLED_BLINK_TICKS;
  }
  led_blinking_tick--;
}
