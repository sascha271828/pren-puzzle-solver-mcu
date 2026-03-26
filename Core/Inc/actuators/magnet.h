#ifndef __MAGNET_H__
#define __MAGNET_H__

#include "sys_config.h"
#include "utils.h"
#include <stdbool.h>

/**
 * @file magnet.h
 * @brief Thin driver for a GPIO-controlled electromagnet.
 *
 * Wraps a single output pin behind a simple on/off API.
 * The module holds one internal pointer to the active GPIO pin,
 * so only one magnet instance is supported at a time.
 *
 * Typical usage:
 * @code
 *   Magnet_Init(&magnetPin);
 *   Magnet_Grab(true);   // engage
 *   Magnet_Grab(false);  // release
 * @endcode
 */

/**
 * @brief Initialises the magnet driver with the GPIO pin to control.
 *        Must be called before Magnet_Grab() or Magnet_GetState().
 *
 * @param pin  GPIO_Pin_t of the output pin.
 */
void Magnet_Init(GPIO_Pin_t pin);

/**
 * @brief Engages or releases the magnet.
 *
 * @param state  true  = activate magnet (GPIO high).
 *               false = release magnet (GPIO low).
 */
void Magnet_SetState(bool state);

/**
 * @brief Returns the current drive state of the magnet pin.
 *
 * @return true   Pin is driven high (magnet active).
 * @return false  Pin is driven low  (magnet inactive).
 */
bool Magnet_GetState(void);

#endif /* __MAGNET_H__ */