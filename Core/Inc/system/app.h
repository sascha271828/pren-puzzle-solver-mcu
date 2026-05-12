#ifndef __APP_H__
#define __APP_H__

/**
 * @file app.h
 * @brief Application entry point and ISR callbacks.
 *
 * App_Run() is the top-level function called from main() after hardware
 * init. Behaviour is selected at compile time via RUN_MODE in sys_config.h:
 *
 *  - RUN_MODE_APP        : production loop — polls CommandDispatcher and
 *                          calls StateMachine_Update() forever (no return).
 *  - RUN_MODE_TEST_CLI   : interactive UART CLI for subsystem testing
 *                          (no return).
 *  - RUN_MODE_TEST_STATE : single-shot state machine test with injected data
 *                          (returns when the test sequence completes).
 *  - RUN_MODE_LED        : LED toggle test driven by Start/Reset buttons
 *                          (no return).
 *
 * The TIM2 period-elapsed callback (HAL_TIM_PeriodElapsedCallback) also
 * lives here and dispatches to the correct ISR handlers based on
 * Interrupt_GetState().
 */

#include "sys_config.h"

/**
 * @brief Starts the application. Returns only in RUN_MODE_TEST_STATE;
 *        loops forever in all other modes.
 */
void App_Run(void);

#endif /* __APP_H__ */