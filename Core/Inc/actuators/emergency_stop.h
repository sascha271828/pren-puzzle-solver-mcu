#ifndef __EMERGENCY_STOP_H__
#define __EMERGENCY_STOP_H__

/**
 * @file emergency_stop.h
 * @brief Emergency-stop input monitoring.
 *
 * Monitors a dedicated digital input (DIN_9, active-high) for an
 * emergency-stop condition.  When triggered, EmergencyStop_Process()
 * immediately transitions the system state machine to IS_ESTOP via
 * Interrupt_SetState(), halting all further motion.
 *
 * Two usage patterns are supported:
 *  - Reactive: call EmergencyStop_Process() periodically (e.g. from the
 *    main loop or a low-priority ISR) to let the module drive the state
 *    transition automatically.
 *  - Polling: call EmergencyStop_IsActivated() to query the raw GPIO level
 *    without side effects, e.g. for startup checks or higher-level logic.
 */

#include "sys_config.h"
#include "utils.h"

#include <stdbool.h>

/**
 * @brief Reads the emergency-stop GPIO and, if active, transitions the
 *        system to IS_ESTOP.
 *
 * Intended to be called periodically.  Has no effect when the input is
 * inactive.
 */
void EmergencyStop_Process(void);

/**
 * @brief Returns the instantaneous state of the emergency-stop input.
 *
 * Does not modify any state; safe to call from any context.
 *
 * @return true   Emergency-stop input is currently active (GPIO_PIN_SET).
 * @return false  Input is inactive; normal operation may continue.
 */
bool EmergencyStop_IsActivated(void);

#endif /* __EMERGENCY_STOP_H__ */