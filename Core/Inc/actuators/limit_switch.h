#ifndef __LIMIT_SWITCH_H__
#define __LIMIT_SWITCH_H__

/**
 * @file limit_switch.h
 * @brief GPIO-based limit-switch driver for the X/Y axes.
 *
 * Manages four limit switches (X_MIN, X_MAX, Y_MIN, Y_MAX) via direct GPIO
 * reads.  The active state of all switches is returned as a bitmask using
 * the LimitSwitches_e flags, allowing callers to test individual switches
 * or combinations with bitwise AND.
 *
 * Example:
 * @code
 *   if (LimitSwitch_Activated() & LIM_X_MIN) {
 *       // X-axis has reached its minimum limit
 *   }
 * @endcode
 *
 * Switch indices in the internal array:
 *  [0] = X_MIN, [1] = X_MAX, [2] = Y_MIN, [3] = Y_MAX
 */

#include "sys_config.h"
#include "utils.h"

/** @brief Total number of limit switches managed by this module. */
#define LIMIT_SWITCH_NUM 4

/**
 * @brief Bitmask flags for individual limit switch states.
 *
 * Values are powers of two so they can be combined freely:
 * @code
 *   uint32_t active = LimitSwitch_Activated();
 *   if ((active & (LIM_X_MIN | LIM_Y_MIN)) == (LIM_X_MIN | LIM_Y_MIN)) {
 *       // both min switches are active simultaneously
 *   }
 * @endcode
 */
typedef enum {
  LIM_NO_LIM = 0,     /**< No switch active                             */
  LIM_X_MIN = BIT(0), /**< X-axis minimum limit switch active           */
  LIM_X_MAX = BIT(1), /**< X-axis maximum limit switch active           */
  LIM_Y_MIN = BIT(2), /**< Y-axis minimum limit switch active           */
  LIM_Y_MAX = BIT(3), /**< Y-axis maximum limit switch active           */
} LimitSwitches_e;

/**
 * @brief Registers the GPIO pins for all four limit switches.
 *        Must be called once during system initialisation before any call
 *        to LimitSwitch_Activated().
 *
 * @param x_min  GPIO pin descriptor for the X-axis minimum switch.
 * @param x_max  GPIO pin descriptor for the X-axis maximum switch.
 * @param y_min  GPIO pin descriptor for the Y-axis minimum switch.
 * @param y_max  GPIO pin descriptor for the Y-axis maximum switch.
 */
void LimitSwitch_Init(GPIO_Pin_t x_min,
                      GPIO_Pin_t x_max,
                      GPIO_Pin_t y_min,
                      GPIO_Pin_t y_max);

/**
 * @brief Reads all four limit-switch GPIOs and returns their combined state.
 *
 * Each bit in the return value corresponds to one switch flag defined in
 * LimitSwitches_e.  A bit is set when the corresponding GPIO reads high
 * (GPIO_PIN_SET).
 *
 * @return uint32_t  Bitmask of currently active switches (LimitSwitches_e).
 *                   Returns LIM_NO_LIM (0) when no switch is active.
 */
uint32_t LimitSwitch_Activated(void);

#endif /* __LIMIT_SWITCH_H__ */