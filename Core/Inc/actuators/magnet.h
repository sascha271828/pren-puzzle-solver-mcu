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
 * @brief Returns the raw GPIO readback of the magnet output pin.
 *
 * @note  Due to hardware polarity inversion in Magnet_SetState() (a switch
 *        inverts the signal), a return value of true (pin high) does NOT
 *        necessarily mean the magnet is active. Use Magnet_SetState() as the
 *        authoritative source for the intended logical state.
 *
 * @return true   GPIO pin reads high.
 * @return false  GPIO pin reads low.
 */
bool Magnet_GetState(void);

#endif /* __MAGNET_H__ */