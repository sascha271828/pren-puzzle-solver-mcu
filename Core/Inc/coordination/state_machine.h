#ifndef __STATE_MACHINE_H__
#define __STATE_MACHINE_H__

#include "command_dispatcher.h"
#include "magnet.h"
#include "piston.h"
#include "rotator.h"

/**
 * @file state_machine.h
 * @brief Non-blocking state machine orchestrating the puzzle-solving workflow.
 *
 * This module controls the high-level sequence of picking, moving, rotating,
 * and placing puzzle pieces. It ties together the communication
 * (CommandDispatcher), motion planning, and actuator control (Piston, Magnet,
 * Rotator) into a continuous, non-blocking execution loop.
 */

/**
 * @brief Initializes the coordination state machine.
 * Stores references to the required hardware drivers and prepares
 * the initial idle state.
 *
 * @param dispatcher Pointer to the command dispatcher handling incoming PC
 * data.
 * @param piston     Pointer to the piston (Z-axis) driver.
 * @param magnet     Pointer to the electromagnet driver.
 * @param rotator    Pointer to the rotational stepper driver.
 */
void StateMachine_Init(CommandDispatcher_t* dispatcher,
                       Piston_t* piston,
                       Magnet_t* magnet,
                       Rotator_t* rotator);

/**
 * @brief Main update routine of the state machine.
 * Evaluates current hardware states (e.g., IsBusy flags) and triggers
 * subsequent actions based on the active puzzle command.
 * * @note  Must be called cyclically from the main application loop. It is
 * strictly non-blocking.
 */
void StateMachine_Update(void);

#endif /* __STATE_MACHINE_H__ */