#ifndef __PISTON_H__
#define __PISTON_H__

#include "sys_config.h"
#include "utils.h"

#include <stdbool.h>

/**
 * @file piston.h
 * @brief Non-blocking piston driver for a DRV8871 H-bridge actuator.
 *
 * Tracks the piston at a single logical level (application positions).
 * Motion is time-based: Piston_Set() starts a move and returns immediately.
 * The caller must drive the state machine by calling Piston_Update()
 * periodically from a timer ISR.
 *
 * On Init, the piston assumes worst-case (fully extended / RELEASE position)
 * and immediately begins retracting to START. The system is considered ready
 * once Piston_IsBusy() returns false.
 *
 * Valid transitions (all directions):
 *   START ↔ MOVE ↔ GRAB
 *                ↔ RELEASE
 *   Direct shortcuts (START ↔ GRAB, START ↔ RELEASE, GRAB ↔ RELEASE)
 *   are supported via precomputed tick counts in the transition table.
 *
 * Typical usage:
 * @code
 *   Piston_Init(pin_extend, pin_retract);
 *   while (Piston_IsBusy()) {}          // wait for init retract
 *
 *   Piston_Set(PISTON_POS_GRAB);
 *   while (Piston_IsBusy()) {}
 * @endcode
 *
 * Compile-time configuration (sys_config.h):
 *   TIMER_FREQ_HZ_ACTUATORS          — ISR tick rate [Hz]
 *   CONFIG_PISTON_TIME_*_MS          — per-segment travel times [ms]
 */

/**
 * @brief Application-level positions the piston can occupy.
 *
 * Enum order determines the direction heuristic in Piston_Set():
 * target > current → extend, target < current → retract.
 * Do not reorder without updating the transition table layout.
 *
 *   START   : fully retracted home position.
 *   MOVE    : intermediate travel height, safe for XY motion.
 *   GRAB    : extended to pick-up depth.
 *   RELEASE : extended to place depth.
 */
typedef enum {
  PISTON_POS_START = 0,
  PISTON_POS_MOVE,
  PISTON_POS_GRAB,
  PISTON_POS_RELEASE,
  PISTON_POS_COUNT /**< Sentinel — number of states, used for table sizing */
} PistonLogical_e;

/**
 * @brief Internal state of a piston instance.
 *
 * Not intended for direct access by callers. Exposed in the header only
 * because the singleton lives in .c — treat all fields as private.
 *
 * @note Fields written from the TIM2 ISR (Piston_Update) are marked volatile.
 */
typedef struct {
  volatile bool is_moving;          /**< True while a move is in progress   */
  volatile PistonLogical_e current; /**< Last confirmed logical position     */
  volatile PistonLogical_e target;  /**< Logical goal of the current move    */
  volatile int32_t ticks_until;     /**< Countdown to move completion        */
  GPIO_Pin_t piston_extend;         /**< H-bridge extend output pin      */
  GPIO_Pin_t piston_retract;        /**< H-bridge retract output pin     */
} Piston_t;

/**
 * @brief Initialises the piston driver and begins homing retract.
 *
 * Assumes the piston may be fully extended (worst case). Immediately
 * energises the retract direction and runs for CONFIG_PISTON_TICKS_RETRACT_INIT
 * ticks before declaring position START.
 *
 * @param pin_extend   GPIO pin connected to H-bridge IN1 (extend direction).
 * @param pin_retract  GPIO pin connected to H-bridge IN2 (retract direction).
 */
void Piston_Init(GPIO_Pin_t pin_extend, GPIO_Pin_t pin_retract);

/**
 * @brief Requests a move to a new logical position.
 *
 * Silently ignored if a move is already in progress or if already at target.
 * Direction (extend/retract) is derived from enum ordering — see
 * PistonLogical_e. Travel time is taken from the precomputed transition table.
 *
 * @param target_pos  Desired logical position.
 */
void Piston_Set(PistonLogical_e target_pos);

/**
 * @brief Immediately cuts power to both H-bridge outputs.
 *
 * Sets current and target to PISTON_POS_RELEASE (conservative unknown).
 * Call Piston_Init() or manually drive to a known position before
 * relying on Piston_Set() again.
 */
void Piston_Abort(void);

/**
 * @brief Returns whether a move is currently in progress.
 *
 * @return true   Move in progress; Piston_Set() calls will be ignored.
 * @return false  Piston is idle and ready for next command.
 */
bool Piston_IsBusy(void);

/**
 * @brief Advances the piston state machine. Call from timer ISR.
 *
 * Decrements the internal tick counter. When it reaches zero, de-energises
 * the H-bridge and updates current position to target.
 * No-op if no move is in progress.
 */
void Piston_Update(void);

#endif /* __PISTON_H__ */