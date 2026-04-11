#ifndef __BUTTONS_H__
#define __BUTTONS_H__

#include "sys_config.h"

#include <stdbool.h>

bool Buttons_Start_Pressed(void);
void Buttons_Start_RearmPressDetection(
    void); /* buttons tracks if pressed once -> so it doens't have to be polled
              the whole time -> reset the keep tracking so a new start can be
              recorded*/
void Buttons_Poll_ISR(void);

#endif /* __BUTTONS_H__ */