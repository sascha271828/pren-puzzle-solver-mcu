#ifndef __BUTTONS_H__
#define __BUTTONS_H__

/**
 * @file buttons.h
 * @brief Button polling and press-detection for the puzzle solver.
 *
 * Tracks latching press events for the Start (DIN_10) and Reset (DIN_12)
 * buttons.  Both flags are set inside the shared timer ISR via
 * Buttons_Poll_ISR() and remain set until the caller explicitly rearms them.
 * This avoids continuous GPIO polling in the main loop.
 *
 * Hardware:
 *   - Start button: DIN_10, active-low (GPIO_PIN_RESET = pressed)
 *   - Reset button: DIN_11, active-low (GPIO_PIN_RESET = pressed)
 */

#include "sys_config.h"

#include <stdbool.h>

/**
 * @brief Returns whether the start button has been pressed since the last
 *        call to Buttons_Start_RearmPressDetection().
 *
 * @return true   Button was pressed at least once since last rearm.
 * @return false  No press detected.
 */
bool Buttons_Start_Pressed(void);

/**
 * @brief Clears the latched start-button press flag.
 *
 * Call this after consuming a press event to re-enable detection.
 * Without this call, Buttons_Start_Pressed() will keep returning true
 * regardless of subsequent button activity.
 */
void Buttons_Start_RearmPressDetection(void);

/**
 * @brief Returns whether the reset button has been pressed since the last
 *        call to Buttons_Reset_RearmPressDetection().
 *
 * @return true   Button was pressed at least once since last rearm.
 * @return false  No press detected.
 */
bool Buttons_Reset_Pressed(void);

/**
 * @brief Clears the latched reset-button press flag.
 *
 * Call this after consuming a press event to re-enable detection.
 * Without this call, Buttons_Reset_Pressed() will keep returning true
 * regardless of subsequent button activity.
 */
void Buttons_Reset_RearmPressDetection(void);

/**
 * @brief Polls both button GPIOs and latches any active press.
 *        Must be called from the shared timer ISR on every tick.
 *
 * @note  Not to be called from any other context.
 */
void Buttons_Poll_ISR(void);

#endif /* __BUTTONS_H__ */