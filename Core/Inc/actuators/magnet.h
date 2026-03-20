#ifndef __MAGNET_H__
#define __MAGNET_H__

#include "sys_config.h"
#include "utils.h"

#include <stdbool.h>

void Magnet_Init(GPIO_Pin_t *pin);

void Magnet_Grab(bool state);

#endif /* __MAGNET_H__ */