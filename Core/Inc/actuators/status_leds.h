/**
 * @file status_leds.h
 * @brief Status LED driver for visual system state indication
 *
 * @details
 * Provides control over three hardware status LEDs (green, yellow, red) with
 * support for both static on/off and blinking modes. Blinking is driven via
 * ISR callback to maintain timing accuracy without blocking main loop.
 *
 * **Hardware mapping:**
 * - Green LED:  DOUT_5 (port/pin defined in main.h)
 * - Yellow LED: DOUT_6 (port/pin defined in main.h)
 * - Red LED:    DOUT_7 (port/pin defined in main.h)
 *
 * **Blink frequency configuration:**
 * Controlled by `STATUSLED_BLINK_FREQUENCY` in sys_config.h (in mHz).
 * Blink period = 1 / frequency
 *
 * **Reentrancy protection:**
 * StatusLeds_On() and StatusLeds_Blink() use TIM2 interrupt masking to
 * ensure atomic updates to shared state (led_blinking_mode,
 * led_blinking_index). Safe to call from main context while
 * StatusLeds_Blink_ISR() runs in TIM2 ISR.
 *
 * Usage pattern:
 * @code
 * // One-time init
 * StatusLeds_Init();
 *
 *
 * // In ISR (e.g., TIM2 at 120 kHz)
 * void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
 *   if (htim == &htim2) {
 *     StatusLeds_Blink_ISR();  // Update blink state
 *   }
 * }
 * @endcode
 *
 * State machine (implicit):
 * @code
 * [STATIC_OFF] ──StatusLeds_On()──> [STATIC_ON]
 *      │                                 │
 *      └──StatusLeds_Blink()──> [BLINKING] <──┘
 *                                  │
 *                              (ISR toggles @ STATUSLED_BLINK_FREQUENCY)
 * @endcode
 *
 * @author Sascha (PREN2 team)
 * @date 2025
 */

#ifndef __STATUS_LEDS_H__
#define __STATUS_LEDS_H__

#include "sys_config.h"
#include "utils.h"

/**
 * @brief Status LED identifier
 *
 * Maps to physical hardware outputs (DOUT_5, DOUT_6, DOUT_7).
 */
typedef enum {
  STATUSLED_GREEN = 0, /**< Green LED (DOUT_5) - "puzzle done" */
  STATUSLED_YELLOW,    /**< Yellow LED (DOUT_6) - "ready / completing" */
  STATUSLED_RED,       /**< Red LED (DOUT_7) - "emergency stop activated" */
  STATUSLED_SENTINEL   /**< Array size sentinel, not a valid LED */
} StatusLeds_e;

/**
 * @brief LED operation mode (currently unused in API)
 *
 * Defined for future expansion but not actively used in current implementation.
 * Actual mode is implicitly set by calling StatusLeds_On() vs
 * StatusLeds_Blink().
 */
typedef enum {
  STATUSLED_TYPE_OFF,   /**< LED off */
  STATUSLED_TYPE_BLINK, /**< LED blinking (ISR-driven) */
  STATUSLED_TYPE_ON,    /**< LED static on */
} StatusLeds_Type_e;

/**
 * @brief Initialize status LED subsystem
 *
 * Turns off all LEDs and prepares GPIO states. Must be called once
 * during system initialization before any StatusLeds_On/Blink calls.
 *
 * @pre GPIO clocks and pin modes configured by CubeMX/main.c
 * @post All LEDs off, blinking mode disabled
 */
void StatusLeds_Init(void);

/**
 * @brief Start blinking a specific LED
 *
 * @details
 * Enters blinking mode for the specified LED. All other LEDs are turned off.
 * The blink timing is controlled by StatusLeds_Blink_ISR() which must be
 * called periodically from an ISR (typically TIM2 at 120 kHz).
 *
 * **Thread safety:** Uses TIM2 interrupt masking to atomically update shared
 * state. Safe to call from main context while ISR is active.
 *
 * @param[in] led  LED to blink (STATUSLED_GREEN/YELLOW/RED)
 *
 * @pre StatusLeds_Init() called
 * @post Specified LED blinking, others off, blinking mode active
 *
 * @see StatusLeds_Blink_ISR()
 */
void StatusLeds_Blink(StatusLeds_e led);

/**
 * @brief Turn on a specific LED (static)
 *
 * @details
 * Sets the specified LED to static on, turns off all other LEDs,
 * and disables blinking mode if it was active.
 *
 * **Thread safety:** Uses TIM2 interrupt masking to atomically disable
 * blinking mode. Safe to call from main context while ISR is active.
 *
 * @param[in] led  LED to turn on (STATUSLED_GREEN/YELLOW/RED)
 *
 * @pre StatusLeds_Init() called
 * @post Specified LED on, others off, blinking mode disabled
 */
void StatusLeds_On(StatusLeds_e led);

/**
 * @brief ISR callback to update LED blink state
 *
 * @details
 * Must be called periodically from an interrupt context to drive the
 * blink timing. Typically invoked from TIM2 period elapsed callback
 * at 120 kHz (8.3 µs period). Does nothing if blinking mode is not active.
 *
 * The blink period is determined by `STATUSLED_BLINK_FREQUENCY` in sys_config.h
 * (in mHz). Internal tick counter decrements each call; when it reaches zero,
 * the LED toggles and the counter reloads with `STATUSLED_BLINK_TICKS`.
 *
 * **Example timing:**
 * - STATUSLED_BLINK_FREQUENCY = 1000 mHz (1 Hz)
 * - At 120 kHz ISR rate: 120,000 ticks per ON phase, 120,000 ticks per OFF
 * phase
 * - Total period = 1 second (500 ms ON, 500 ms OFF)
 *
 * ⚠ Must be called from ISR context for timing accuracy.
 *
 * @pre StatusLeds_Blink() called to enter blinking mode
 * @post LED state toggled when tick counter expires (if blinking active)
 *
 * @see StatusLeds_Blink()
 * @see STATUSLED_BLINK_FREQUENCY in sys_config.h
 */
void StatusLeds_Blink_ISR(void);

#endif /* __STATUS_LEDS_H__ */