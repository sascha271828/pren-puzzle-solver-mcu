#ifndef __LIMIT_SWITCH_H__
#define __LIMIT_SWITCH_H__

#include "sys_config.h"
#include "utils.h"

#define LIMIT_SWITCH_NUM 4

typedef enum {
  LIM_X_MIN = (1 << 0),
  LIM_X_MAX = (1 << 1),
  LIM_Y_MIN = (1 << 2),
  LIM_Y_MAX = (1 << 3),
} LimitSwitches_e;



void LimitSwitch_Init(GPIO_Pin_t x_min,
                      GPIO_Pin_t x_max,
                      GPIO_Pin_t y_min,
                      GPIO_Pin_t y_max);

uint32_t LimitSwitch_Activated(void);

#endif /* __LIMIT_SWITCH_H__ */