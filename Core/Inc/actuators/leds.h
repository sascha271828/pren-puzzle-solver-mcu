#ifndef __LEDS_H__
#define __LEDS_H__

/**
 * @file leds.h
 * @brief Single-LED driver for the puzzle solver status indicator.
 *
 * Wraps a single GPIO-driven LED behind a logical active-high API.
 * The hardware polarity inversion (active-low GPIO) is handled internally,
 * so callers always use true = on, false = off.
 *
 * Usage:
 * @code
 *   Leds_Init((GPIO_Pin_t){ .port = LED_GPIO_Port, .pin = LED_Pin });
 *   Leds_Set(true);   // turn on
 *   Leds_Set(false);  // turn off
 * @endcode
 */

#include "sys_config.h"
#include "utils.h"

#include <stdbool.h>

/**
 * @brief Initialises the LED driver and turns the LED on.
 *
 * Stores the GPIO descriptor for subsequent calls and drives the pin to the
 * on state.  Must be called before Leds_Set() or Leds_Get().
 *
 * @param pin  GPIO port/pin descriptor for the LED output.
 */
void Leds_Init(GPIO_Pin_t pin);

/**
 * @brief Sets the LED to the requested logical state.
 *
 * Internally inverts the level to account for the active-low hardware wiring.
 *
 * @param state  true to turn the LED on, false to turn it off.
 */
void Leds_Set(bool state);

/**
 * @brief Returns the current logical state of the LED.
 *
 * Reads back the GPIO pin and inverts the level to account for the
 * active-low hardware wiring, so the return value matches the value
 * last written with Leds_Set().
 *
 * @return true   LED is on.
 * @return false  LED is off.
 */
bool Leds_Get(void);

#endif /* __LEDS_H__ */