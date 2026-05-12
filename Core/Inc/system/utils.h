#ifndef __UTILS_H__
#define __UTILS_H__

/**
 * @file utils.h
 * @brief Shared utility types used across all modules.
 */

#include "stm32h753xx.h"

#include <stdint.h>

/**
 * @brief Bundles a GPIO port pointer and pin mask into a single descriptor.
 *        Used by all actuator and sensor modules to identify an I/O pin.
 */
typedef struct {
  GPIO_TypeDef *port; /**< Pointer to the GPIO peripheral (e.g. GPIOA) */
  uint16_t pin;       /**< HAL pin bitmask (e.g. GPIO_PIN_5)            */
} GPIO_Pin_t;

#endif /* __UTILS_H__ */