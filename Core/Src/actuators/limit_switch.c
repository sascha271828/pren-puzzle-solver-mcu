#include "main.h"

#include "limit_switch.h"

static struct {
  GPIO_Pin_t limit[LIMIT_SWITCH_NUM];
} LimitSwitch_Pins;

void LimitSwitch_Init(GPIO_Pin_t x_min,
                      GPIO_Pin_t x_max,
                      GPIO_Pin_t y_min,
                      GPIO_Pin_t y_max) {
  LimitSwitch_Pins.limit[0] = x_min;
  LimitSwitch_Pins.limit[1] = x_max;
  LimitSwitch_Pins.limit[2] = y_min;
  LimitSwitch_Pins.limit[3] = y_max;
}

uint32_t LimitSwitch_Activated(void) {
  uint32_t lim = 0;
  if (HAL_GPIO_ReadPin(LimitSwitch_Pins.limit[0].port,
                       LimitSwitch_Pins.limit[0].pin)) {
    lim |= LIM_X_MIN;
  }

  if (HAL_GPIO_ReadPin(LimitSwitch_Pins.limit[1].port,
                       LimitSwitch_Pins.limit[1].pin)) {
    lim |= LIM_X_MAX;
  }
  if (HAL_GPIO_ReadPin(LimitSwitch_Pins.limit[2].port,
                       LimitSwitch_Pins.limit[2].pin)) {
    lim |= LIM_Y_MIN;
  }
  if (HAL_GPIO_ReadPin(LimitSwitch_Pins.limit[3].port,
                       LimitSwitch_Pins.limit[3].pin)) {
    lim |= LIM_Y_MAX;
  }

  return lim;
}
