#ifndef __MAGNET_H__
#define __MAGNET_H__

#include "sys_config.h"
#include "utils.h"
#include <stdbool.h>

/**
 * @file magnet.h
 * @brief ISR-driven electromagnet driver with full-power delay, PWM hold, and
 *        auto-timeout.
 *
 * Activation follows a three-phase sequence managed by Magnet_Process():
 *   1. Full-power phase: magnet is energised at 100 % for
 *      CONFIG_MAGNET_DELAY_MS to ensure reliable pick-up.
 *   2. PWM hold phase: duty cycle is reduced to CONFIG_MAGNET_PWM_ENUMERATER /
 *      CONFIG_MAGNET_PWM_DIVISOR to limit coil heating during transport.
 *   3. Auto-timeout: if the magnet remains active for more than
 *      CONFIG_MAGNET_TIMEOUT_MS, it is deactivated automatically.
 *
 * Magnet_Process() must be called from a fixed-rate timer ISR
 * (TIMER_FREQ_HZ_ACTUATORS). Magnet_SetState() may be called from the
 * main-loop context.
 *
 * @note  The GPIO output is hardware-inverted (a transistor switch reverses
 *        polarity). All API functions use the logical state (true = active).
 *
 * Typical usage:
 * @code
 *   Magnet_Init(magnetPin);
 *   Magnet_SetState(true);   // engage — full-power then PWM hold
 *   // ... transport piece ...
 *   Magnet_SetState(false);  // release immediately
 * @endcode
 */

/**
 * @brief Initialises the magnet driver with the GPIO pin to control.
 *        Must be called once at system start before any other Magnet_* call.
 *
 * @param pin  GPIO_Pin_t of the output pin connected to the magnet switch.
 */
void Magnet_Init(GPIO_Pin_t pin);

/**
 * @brief Engages or releases the magnet.
 *
 * Calling with state = true resets the internal delay and PWM counters and
 * begins the full-power phase. Calling with state = false immediately
 * de-energises the coil regardless of which phase is active.
 *
 * @param state  true  = activate magnet (begin full-power + PWM sequence).
 *               false = deactivate magnet immediately.
 */
void Magnet_SetState(bool state);

/**
 * @brief Advances the magnet PWM state machine. Must be called from the
 *        timer ISR at TIMER_FREQ_HZ_ACTUATORS.
 *
 * Manages the full-power delay, PWM hold phase, and auto-timeout. No-op
 * when the magnet is inactive.
 */
void Magnet_Process(void);

/**
 * @brief Returns the logical activation state of the magnet.
 *
 * @return true   Magnet is currently active (full-power or PWM hold phase).
 * @return false  Magnet is inactive (de-energised).
 */
bool Magnet_GetState(void);

#endif /* __MAGNET_H__ */