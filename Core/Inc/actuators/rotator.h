#ifndef __ROTATION_H__
#define __ROTATION_H__

#include "stepper.h"

/**
 * @file rotation.h
 * @brief Single-axis rotation driver with trapezoidal speed profile.
 *
 * Drives one stepper motor at maximum microstepping resolution.
 * The speed profile (acceleration, cruise, deceleration) is precomputed
 * once in Rotator_GenerateBlock() and stored in a lookup table.
 * Execution is non-blocking: Rotator_StartMove() returns immediately and
 * Rotator_Update() must be called from a shared timer ISR each tick.
 *
 * Position is tracked internally in steps and can be reset to zero via
 * Rotator_ReturnStart().
 *
 * Compile-time parameters (define in sys_config.h):
 *  - MAX_ACCEL_STEPS_ROT        – maximum entries in the ramp table
 *  - CONFIG_STEPS_PER_01_DEGREE – motor resolution [steps / 0.1°]
 *  - CONFIG_MAX_SPEED_ROT       – cruise speed     [°/s]
 *  - CONFIG_ACCEL_AXIS_ROT      – acceleration     [°/s²]
 *  - TIMER_FREQ_HZ_STEP         – ISR tick frequency [Hz]
 */

/**
 * @brief Precomputed timer-tick intervals for the acceleration ramp.
 *        Entry [n] is the number of ISR ticks to wait before step n.
 *        The same table is mirrored for the deceleration phase.
 */
typedef struct {
  uint32_t interval[ROT_ACCEL_STEPS_IDEAL];
} interval_table_rot_t;

/**
 * @brief Fully describes a single rotation move including its speed profile.
 *        Produced by Rotator_GenerateBlock() and treated as read-only
 *        once passed to Rotator_StartMove().
 *
 * Profile layout by step index:
 *
 *  speed
 *   |      ___________
 *   |     /           \
 *   |    /             \
 *   |___/               \___
 *   0  ^accel_until ^decel_at ^path_steps
 *
 *  0 … accel_until   acceleration  (interval_table lookup)
 *  accel_until … decel_at  cruise  (cruise_interval)
 *  decel_at … path_steps   decel   (interval_table mirrored)
 */
typedef struct {
  int32_t steps;            /**< Signed step count (negative = reverse)     */
  uint32_t accel_until;     /**< Step index where acceleration ends         */
  uint32_t decel_at;        /**< Step index where deceleration begins       */
  uint32_t cruise_interval; /**< Constant ISR-tick interval during cruise   */
  uint32_t path_steps;      /**< Total steps to execute (always positive)   */
  interval_table_rot_t interval_table; /**< Precomputed ramp intervals       */
  uint32_t table_len; /**< Number of valid entries in interval_table  */
} RotateBlock_t;

/**
 * @brief Initialises the rotator with the stepper motor instance to drive.
 *        Must be called once before any other function.
 *
 * @param rot  Pointer to the Stepper_t instance for the rotation axis.
 */
void Rotator_Init(Stepper_t* rot);

/**
 * @brief Computes a fully populated RotateBlock_t for the requested move,
 *        including the trapezoidal speed profile and interval lookup table.
 *        Falls back to a triangular profile when the move is too short to
 *        reach cruise speed.
 *
 * @note  steps == 0 produces an undefined block; do not pass to StartMove.
 *
 * @param steps  Signed number of steps to rotate (negative = reverse).
 * @return RotateBlock_t  Fully initialised, ready-to-execute move block.
 */
RotateBlock_t Rotator_GenerateBlock(int32_t steps);

/**
 * @brief Loads a move block and starts execution. Returns immediately.
 *        The caller must keep the block alive until Rotator_IsBusy()
 *        returns false.
 *
 * @param block  Pointer to a RotateBlock_t from Rotator_GenerateBlock().
 * @return true   Move accepted and started.
 * @return false  Rejected — a move is already in progress.
 */
bool Rotator_StartMove(const RotateBlock_t* block);

/**
 * @brief Generates and starts a move back to the zero position.
 *        Uses an internal static block; no external storage needed.
 *        Has no effect if current position is already zero.
 *
 * @return true   Homing move started (or already at zero).
 * @return false  Rejected — a move is already in progress.
 */
bool Rotator_ReturnStart(void);

/**
 * @brief Returns whether a move is currently in progress.
 *
 * @return true   Motor is running; do not call Rotator_StartMove().
 * @return false  Rotator is idle and ready for a new block.
 */
bool Rotator_IsBusy(void);

/**
 * @brief Advances the rotator state machine by one ISR tick.
 *        Must be called from the shared timer ISR on every tick.
 *        Handles pulse clearing, step timing, interval lookup,
 *        and move completion.
 *
 * @note  Not to be called from any other context.
 */
void Rotator_Update(void);

void Rotator_Abort(void);

#endif /* __ROTATION_H__ */