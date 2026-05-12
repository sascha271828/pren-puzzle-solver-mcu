#ifndef __SYS_INIT_H__
#define __SYS_INIT_H__

/**
 * @file sys_init.h
 * @brief System-level hardware initialisation and singleton accessors.
 *
 * Sys_Init() instantiates all hardware objects (steppers, piston, limit
 * switches, LEDs, magnet, homing, UART/protobuf stack) and wires them
 * together with their GPIO pin assignments. It must be called once from
 * main() before App_Run().
 *
 * Behaviour depends on RUN_MODE (sys_config.h):
 *  - RUN_MODE_LED  : only initialises the single indicator LED and TIM2.
 *  - All other modes: full peripheral init including UART communication
 *    stack in RUN_MODE_APP.
 */

#include "command_dispatcher.h"
#include "piston.h"
#include "sys_config.h"
#include "uart_receiver.h"

/**
 * @brief Initialises all hardware peripherals and module instances.
 *        Must be called once at startup before any other module function.
 */
void Sys_Init(void);

/**
 * @brief Returns a pointer to the singleton UartReceiver instance created
 *        during Sys_Init(). Valid only after Sys_Init() has been called.
 *
 * @return Pointer to the global UartReceiver_t instance.
 */
UartReceiver_t* Sys_GetUartReceiver(void);

/**
 * @brief Returns a pointer to the singleton CommandDispatcher instance
 *        created during Sys_Init(). Valid only after Sys_Init() has been
 *        called.
 *
 * @return Pointer to the global CommandDispatcher_t instance.
 */
CommandDispatcher_t* Sys_GetCommandDispatcher(void);

#endif /* __SYS_INIT_H__ */