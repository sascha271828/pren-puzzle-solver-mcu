#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdint.h>
#include <stdbool.h>

#include "gpio.h"

typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
} GPIO_Pin_t;



#endif