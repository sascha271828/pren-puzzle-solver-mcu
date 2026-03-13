#ifndef __INIT_H__
#define __INIT_H__

#include "utils.h"
#include "sys_config.h"

#include "end_effector.h"
#include "motion_planner.h"
#include "state_machine.h"
#include "stepper.h"


#define SYS_PISTON_SENSOR




/**
 * @brief initializes the system for the State Machine
 * 
 */
void Sys_Init(void);


/**
 * @brief returns the initialized State Machine as a pointer
 * 
 * @return StateMachine_t* the initialized State Machine
 */
 /*
StateMachine_t* Sys_GetMachine(void);
*/





#endif