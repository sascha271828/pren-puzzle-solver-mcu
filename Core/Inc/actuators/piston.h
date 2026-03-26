#ifndef __PISTON_H__
#define __PISTON_H__

#include "sys_config.h"
#include "utils.h"
#include <stdbool.h>

/**
 * @file piston.h
 * @brief Driver for a dual-coil pneumatic piston with optional limit switches.
 *
 * Models the piston at two levels:
 *  - **Physical**: what the hardware is actually doing (extending, retracting,
 *    moving, set, unknown).
 *  - **Logical**: the application-level position the piston represents
 *    (START, MOVE, GRAB, RELEASE).
 *
 * Only valid logical transitions are permitted (enforced by resolve_move).
 * Motion is non-blocking: Piston_Set() starts the move and returns
 * PISTON_BUSY immediately. The caller must call Piston_Update() periodically
 * (from a timer ISR or main-loop tick) to drive the state machine forward.
 *
 * Typical usage:
 * @code
 *   Piston_Init(piston_1_extend, piston_1_retract);
 *
 *   // kick off a move
 *   if (Piston_Set(PISTON_POS_GRAB) == PISTON_BUSY) {
 *     while (Piston_IsBusy()) {
 *       Piston_Update();   // or let ISR handle this
 *     }
 *   }
 * @endcode
 *
 * Compile-time options (define in sys_config.h):
 *  - CONFIG_PISTON_SEPARAT_PINS   – second coil driven by separate GPIO pins
 *  - CONFIG_PISTON_HAS_LIMIT_SWITCH – physical end-stop switches present
 */

/**
 * @brief Low-level hardware state of the piston.
 *
 * UNKNOWN      : initial state after power-on, before first move.
 * EXTENDING    : extend coil energised, piston travelling outward.
 * RETRACTING   : retract coil energised, piston travelling inward.
 * MOVING       : generic in-motion marker (direction known via `target`).
 * SET          : motion complete, coils de-energised, position reached.
 */
typedef enum {
  PISTON_STATE_UNKNOWN = 0,
  PISTON_STATE_EXTENDING,
  PISTON_STATE_RETRACTING,
  PISTON_STATE_MOVING,
  PISTON_STATE_SET,
} PistonPhysical_e;

/**
 * @brief Application-level positions the piston can occupy.
 *
 * UNKNOWN : not yet homed or position undefined.
 * START   : fully retracted home position.
 * MOVE    : intermediate travel height (safe for XY motion).
 * GRAB    : extended down to pick up a piece.
 * RELEASE : extended down to place a piece.
 *
 * Valid transitions:
 *   START   → MOVE
 *   MOVE    → GRAB  | RELEASE | START
 *   GRAB    → MOVE
 *   RELEASE → MOVE
 */
typedef enum {
  PISTON_POS_UNKNOWN = 0,
  PISTON_POS_START,
  PISTON_POS_GRAB,
  PISTON_POS_MOVE,
  PISTON_POS_RELEASE,
} PistonLogical_e;

/**
 * @brief Complete state for one piston instance.
 *
 * Initialise the GPIO pin fields before calling Piston_Init().
 * All volatile fields are written from Piston_Update() which may run in an
 * ISR context.
 */
typedef struct {
  volatile PistonPhysical_e physical;      /**< Current hardware state         */
  volatile PistonLogical_e  logical;       /**< Last successfully reached pos   */
  volatile PistonPhysical_e target;        /**< Physical direction of move      */
  volatile PistonLogical_e  target_logical;/**< Logical goal of current move    */
  uint32_t move_deadline_tick;             /**< HAL_GetTick() timeout threshold */

  GPIO_Pin_t piston_1_extend;              /**< Extend coil output pin          */
  GPIO_Pin_t piston_1_retract;             /**< Retract coil output pin         */

#if CONFIG_PISTON_SEPARAT_PINS
  GPIO_Pin_t piston_2_extend;              /**< Second extend coil (optional)   */
  GPIO_Pin_t piston_2_retract;             /**< Second retract coil (optional)  */
#endif

#if CONFIG_PISTON_HAS_LIMIT_SWITCH
  GPIO_Pin_t limit_switch_extended;        /**< End-stop at full extension      */
  GPIO_Pin_t limit_switch_retracted;       /**< End-stop at full retraction     */
#endif
} Piston_t;

/**
 * @brief Return codes used across the piston API.
 *
 * PISTON_OK                  : operation completed successfully.
 * PISTON_BUSY                : move started, still in progress.
 * PISTON_ERR_BUSY            : rejected — a move is already running.
 * PISTON_ERR_TIMEOUT         : move did not complete within the deadline.
 * PISTON_ERR_INVALID_TRANSITION : requested logical transition is not allowed.
 */
typedef enum {
  PISTON_OK,
  PISTON_BUSY,
  PISTON_ERR_BUSY,
  PISTON_ERR_TIMEOUT,
  PISTON_ERR_INVALID_TRANSITION,
} PistonResult_e;

void Piston_Init(GPIO_Pin_t pin_extend, GPIO_Pin_t pin_retract);


/**
 * @brief Requests a move to a new logical position.
 *        Validates the transition, starts the move, and returns immediately.
 *
 * @param target_pos  Desired logical position.
 * @return PISTON_OK                   if already at target.
 * @return PISTON_BUSY                 if move was successfully started.
 * @return PISTON_ERR_BUSY             if a move is already in progress.
 * @return PISTON_ERR_INVALID_TRANSITION if the requested transition is illegal.
 */
PistonResult_e Piston_Set( PistonLogical_e target_pos);

/**
 * @brief Immediately de-energises all coils and marks state as UNKNOWN.
 *        Safe to call at any time, including mid-move.
 *
 */
void PistonAbort(void);

/**
 * @brief Returns whether a move is currently in progress.
 *
 * @return true   Piston is moving; do not call Piston_Set().
 * @return false  Piston is idle.
 */
bool Piston_IsBusy(void);

/**
 * @brief Advances the piston state machine. Must be called periodically
 *        from a timer ISR or main-loop tick while a move is in progress.
 *
 * Checks limit switches (if enabled) and the move timeout.
 * On completion, updates logical and physical state and de-energises coils.
 *
 */
void Piston_Update(void);

#endif /* __PISTON_H__ */