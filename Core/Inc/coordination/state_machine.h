#ifndef __STATE_MACHINE_H__
#define __STATE_MACHINE_H__

#include "command_dispatcher.h"
#include "magnet.h"
#include "piston.h"
#include "rotator.h"
#include <stdbool.h>


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
 */
void StateMachine_Init(CommandDispatcher_t* dispatcher);

/**
 * @brief Main update routine of the state machine.
 *        Evaluates current hardware states (e.g., IsBusy flags) and triggers
 *        subsequent actions based on the active puzzle command.
 *
 * @note  Must be called cyclically from the main application loop. It is
 *        strictly non-blocking.
 */
void StateMachine_Update(void);

/**
 * @brief Returns whether the state machine is currently idle and waiting
 *        for a new command.
 *
 * @return true   State machine is in SM_IDLE; ready to accept a new command.
 * @return false  A sequence is in progress or the machine is in ESTOP.
 */
bool StateMachine_IsIdle(void);

/**
 * @brief Injects a puzzle command directly, bypassing CommandDispatcher.
 *        Intended for test mode (RUN_MODE_TEST_STATE) only. Starts execution
 *        from the first piece immediately.
 *
 * @param cmd  Pointer to a PuzzleCommand containing at least one piece.
 *             The command is copied internally; the pointer need not remain
 *             valid after this call returns.
 */
void StateMachine_StartManual(PuzzleCommand* cmd);

#endif /* __STATE_MACHINE_H__ */