#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

/**
 * @file interrupt.h
 * @brief Global system-state machine for the puzzle solver.
 *
 * Provides a single shared state variable that coordinates the top-level
 * operating mode of the machine.  All modules (homer, step_generator,
 * emergency_stop, …) read or write this state to signal transitions.
 *
 * State transition diagram (normal flow):
 *
 * @verbatim
 *   [power-on]
 *       |
 *       v
 *   IS_ESTOP ──(E-stop clear + Reset pressed)──► IS_HOMING ──► IS_READY ──► IS_RUNNING
 *       ^                                                                         |
 *       └─────────────────────── (any state → IS_ESTOP on emergency-stop) ───────┘
 * @endverbatim
 *
 * The system always starts in IS_ESTOP on power-on (safe-start mode).
 * All actuators are held idle until the operator clears the E-stop input
 * and presses Reset, which triggers Homer_HomingStart() → IS_HOMING.
 *
 * @note  IS_INIT is retained in the enum for backwards compatibility but is
 *        no longer set by Interrupt_Init() or used in the normal control flow.
 */

/**
 * @brief Top-level operating states of the puzzle-solver machine.
 */
typedef enum {
  IS_ESTOP,   /**< Emergency stop / safe-start — all motion halted           */
  IS_INIT,    /**< Reserved; no longer used in normal flow (see file doc)    */
  IS_READY,   /**< Homing complete; awaiting start command                   */
  IS_HOMING,  /**< Homing sequence in progress                               */
  IS_RUNNING, /**< Puzzle-solving motion sequence executing                  */
} InterruptState_t;

/**
 * @brief Returns the current system state.
 *
 * @return InterruptState_t  The active operating state.
 */
InterruptState_t Interrupt_GetState(void);

/**
 * @brief Initialises the interrupt state to IS_ESTOP (safe-start mode).
 *        Must be called once at system startup before any other module.
 *        All actuators remain idle until a Reset button press transitions
 *        the system to IS_HOMING via Homer_HomingStart().
 */
void Interrupt_Init(void);

/**
 * @brief Unconditionally sets the system state.
 *
 * Any module may call this to signal a state transition.  No guard checks
 * are performed; callers are responsible for valid transition sequencing.
 * The IS_ESTOP state should be set immediately and never overridden.
 *
 * @param is  The new system state to enter.
 */
void Interrupt_SetState(InterruptState_t is);

#endif /* __INTERRUPT_H__ */