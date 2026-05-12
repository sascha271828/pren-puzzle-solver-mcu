#ifndef __SYS_INIT_H__
#define __SYS_INIT_H__

#include "command_dispatcher.h"
#include "piston.h"
#include "sys_config.h"
#include "uart_receiver.h"

#define SYS_PISTON_SENSOR

/**
 * @brief initializes the system for the State Machine
 *
 */
void Sys_Init(void);

UartReceiver_t* Sys_GetUartReceiver(void);
CommandDispatcher_t* Sys_GetCommandDispatcher(void);

#endif /* __SYS_INIT_H__ */