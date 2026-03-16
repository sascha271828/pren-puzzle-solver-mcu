#ifndef __SYS_INIT_H__
#define __SYS_INIT_H__

#include "piston.h"
#include "sys_config.h"


#define SYS_PISTON_SENSOR

/**
 * @brief initializes the system for the State Machine
 *
 */
void Sys_Init(void);

Piston_t* Sys_GetPiston(void);

/**
 * @brief returns the initialized State Machine as a pointer
 *
 * @return StateMachine_t* the initialized State Machine
 */
/*
StateMachine_t* Sys_GetMachine(void);
*/

#endif /* __SYS_INIT_H__ */