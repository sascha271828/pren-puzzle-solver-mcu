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
 *    IS_INIT  ──► IS_HOMING ──► IS_READY ──► IS_RUNNING
 *                                               |
 *   IS_ESTOP ◄─────────────────────────────────┘
 *     (any state → IS_ESTOP on emergency-stop input)
 * @endverbatim
 *
 * @note  IS_ESTOP is a terminal state; recovery requires a full system reset.
 */

/**
 * @brief Top-level operating states of the puzzle-solver machine.
 */
typedef enum {
  IS_ESTOP,   /**< Emergency stop active — all motion halted, terminal state */
  IS_INIT,    /**< System initialising; peripherals not yet ready            */
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
 * @brief Initialises the state machine to IS_INIT.
 *        Must be called once at system startup before any other module.
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