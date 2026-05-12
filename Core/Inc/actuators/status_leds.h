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
 * **Reentrancy note:**
 * StatusLeds_On(), StatusLeds_Off() and StatusLeds_Blink() write directly to
 * shared state (led_blinking_mode[]) without interrupt masking. They are
 * intended to be called from the main context only; concurrent calls from
 * the TIM2 ISR may cause a brief race on the blinking flag.
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
 * @brief Initialize status LED driver and turn all LEDs off
 *
 * Sets all three LEDs to off state and clears any blinking modes.
 * Must be called once during system initialization before using any other
 * StatusLed functions.
 *
 * @pre GPIO pins for DOUT_5/6/7 must be initialized by HAL/CubeMX
 * @post All LEDs off, blinking disabled
 *
 * @note Idempotent — safe to call multiple times
 */
void StatusLeds_Init(void);

/**
 * @brief Read current state/mode of specified LED
 *
 * Returns the LED's current operational mode by checking blinking mode flag
 * first, then reading GPIO pin state if not blinking.
 *
 * Return priority:
 * 1. STATUSLED_TYPE_BLINK if led_blinking_mode[led] is active
 * 2. STATUSLED_TYPE_ON if GPIO pin reads high (and not blinking)
 * 3. STATUSLED_TYPE_OFF if GPIO pin reads low (and not blinking)
 *
 * @param[in] led  LED identifier (STATUSLED_GREEN/YELLOW/RED)
 *
 * @return Current LED mode:
 *         - STATUSLED_TYPE_BLINK: LED in blinking mode (ISR-driven)
 *         - STATUSLED_TYPE_ON: LED static on
 *         - STATUSLED_TYPE_OFF: LED off
 *
 * @pre StatusLeds_Init() must have been called
 *
 * @note If LED is blinking, return value is BLINK regardless of instantaneous
 *       GPIO pin state (on or off phase)
 */
StatusLeds_Type_e StatusLed_Read(StatusLeds_e led);


/**
 * @brief Enable blinking mode for specified LED
 *
 * Activates ISR-driven blinking at the frequency configured by
 * STATUSLED_BLINK_FREQUENCY in sys_config.h. LED immediately turns on, then
 * toggles on/off at each blink interval driven by StatusLeds_Blink_ISR().
 *
 * Multiple LEDs can blink simultaneously — they toggle in sync.
 *
 * @param[in] led  LED identifier (STATUSLED_GREEN/YELLOW/RED)
 *
 * @pre StatusLeds_Init() must have been called
 * @pre TIM2 interrupt must be running and calling StatusLeds_Blink_ISR()
 * @post LED enters blinking mode; led_blinking_mode[led] = true
 *
 * @note To stop blinking, call StatusLeds_On() or StatusLeds_Off()
 * @note Call from main context only; no interrupt masking is applied.
 *
 * @warning Does not validate led parameter — UB if led >= STATUSLED_SENTINEL
 */
void StatusLeds_Blink(StatusLeds_e led);

/**
 * @brief Turn LED on (static, non-blinking)
 *
 * Sets LED to constant-on state. If LED was previously blinking, blinking is
 * disabled and LED remains on.
 *
 * @param[in] led  LED identifier (STATUSLED_GREEN/YELLOW/RED)
 *
 * @pre StatusLeds_Init() must have been called
 * @post LED on; led_blinking_mode[led] = false
 *
 * @note Call from main context only; no interrupt masking is applied.
 * @warning Does not validate led parameter — UB if led >= STATUSLED_SENTINEL
 */
void StatusLeds_On(StatusLeds_e led);

/**
 * @brief Turn LED off
 *
 * Sets LED to off state. If LED was previously blinking, blinking is disabled
 * and LED turns off.
 *
 * @param[in] led  LED identifier (STATUSLED_GREEN/YELLOW/RED)
 *
 * @pre StatusLeds_Init() must have been called
 * @post LED off; led_blinking_mode[led] = false
 *
 * @note Call from main context only; no interrupt masking is applied.
 * @warning Does not validate led parameter — UB if led >= STATUSLED_SENTINEL
 */
void StatusLeds_Off(StatusLeds_e led);

/**
 * @brief ISR callback to update LED blinking state
 *
 * Must be called from TIM2 ISR at 120 kHz (every 8.33 µs). Maintains internal
 * tick counter; toggles all LEDs in blinking mode when counter reaches zero.
 *
 * Timing:
 * - Blink interval = STATUSLED_BLINK_TICKS (derived from
 * STATUSLED_BLINK_FREQUENCY)
 * - All blinking LEDs toggle simultaneously (synchronized)
 *
 * @pre TIM2 running at TIMER_FREQ_HZ_ACTUATORS (120 kHz)
 * @post Decrements led_blinking_tick; toggles LEDs if tick == 0
 *
 * @note Execution time must fit within 8.33 µs ISR budget
 *
 * Usage:
 * @code
 * void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
 *   if (htim == &htim2) {
 *     StatusLeds_Blink_ISR();
 *   }
 * }
 * @endcode
 */
void StatusLeds_Blink_ISR(void);

#endif /* __STATUS_LEDS_H__ */