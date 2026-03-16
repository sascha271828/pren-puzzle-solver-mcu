#ifndef __UTILS_H__
#define __UTILS_H__

#include "stm32h753xx.h"

#include <stdint.h>

typedef struct {
  GPIO_TypeDef *port;
  uint16_t pin;
} GPIO_Pin_t;

#endif /* __UTILS_H__ */