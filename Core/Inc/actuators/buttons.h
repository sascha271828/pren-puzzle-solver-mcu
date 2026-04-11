#ifndef __BUTTONS_H__
#define __BUTTONS_H__

/**
 * @file buttons.h
 * @brief Start-button polling and press-detection for the puzzle solver.
 *
 * Tracks a single latching start-button press.  The detection flag is set
 * inside the timer ISR via Buttons_Poll_ISR() and remains set until the
 * caller explicitly rearms it with Buttons_Start_RearmPressDetection().
 * This avoids the need to poll the GPIO continuously in the main loop.
 *
 * Hardware: DIN_10 (active-high, GPIO_PIN_SET = pressed).
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
 * @brief Clears the latched press flag so the next press can be detected.
 *
 * Call this after consuming a button press event to re-enable detection.
 * Without this call, Buttons_Start_Pressed() will keep returning true
 * regardless of subsequent button activity.
 */
void Buttons_Start_RearmPressDetection(void);

/**
 * @brief Polls the start-button GPIO and latches a press.
 *        Must be called from the shared timer ISR on every tick.
 *
 * @note  Not to be called from any other context.
 */
void Buttons_Poll_ISR(void);

#endif /* __BUTTONS_H__ */