#ifndef __EMERGENCY_STOP_H__
#define __EMERGENCY_STOP_H__

#include "sys_config.h"
#include "utils.h"

#include <stdbool.h>

void EmergencyStop_Process(void);

bool EmergencyStop_IsActivated(void);
#endif /* __EMERGENCY_STOP_H__ */