#ifndef __STEP_GENERATOR_H__
#define __STEP_GENERATOR_H__

#include "stepper.h"
#include "sys_config.h"

/**
 * @brief Precomputed timer-tick intervals for the acceleration/deceleration
 *        ramp. Entry [n] holds the number of ISR ticks to wait before step n
 *        during the ramp. The same table is mirrored for deceleration.
 *        Size is fixed at MAX_ACCEL_STEPS (defined in sys_config.h).
 */
typedef struct {
  uint32_t interval[AXIS_ACCEL_STEPS_IDEAL];
} interval_table_t;

/**
 * @brief Fully describes a single linear move, including its trapezoidal
 *        (or triangular) speed profile. Generated once by
 *        StepGenerator_GenerateBlock() and treated as read-only thereafter.
 *
 * Profile layout (by step index):
 *
 *   0 ... accel_until : acceleration ramp  (interval_table lookup)
 *   accel_until ... decel_at : cruise      (cruise_interval)
 *   decel_at ... path_steps  : decel ramp  (interval_table mirrored)
 *
 *  speed
 *   |      ___________
 *   |     /           \
 *   |    /             \
 *   |___/               \___
 *   0  ^accel_until  ^decel_at  ^path_steps
 */
typedef struct {
  int32_t steps_x;          /**< Signed step count for X axis              */
  int32_t steps_y;          /**< Signed step count for Y axis              */
  uint32_t accel_until;     /**< Step index where acceleration phase ends  */
  uint32_t decel_at;        /**< Step index where deceleration phase begins*/
  uint32_t cruise_interval; /**< Constant ISR-tick interval during cruise  */
  uint32_t path_steps;      /**< Total steps along the dominant axis       */
  interval_table_t interval_table; /**< Precomputed ramp intervals           */
  uint32_t table_len; /**< Number of valid entries in interval_table */
  bool x_dominant;    /**< True if X axis drives the DDA             */
} MoveBlock_t;

/**
 * @brief Initialises the step generator with pointers to the two stepper
 *        motor instances. Must be called once before any other function.
 *
 * @param mx  Pointer to the X-axis Stepper_t instance.
 * @param my  Pointer to the Y-axis Stepper_t instance.
 */
void StepGenerator_Init(Stepper_t* mx, Stepper_t* my);

/**
 * @brief Computes a fully populated MoveBlock_t for the requested move,
 *        including the trapezoidal speed profile and interval lookup table.
 *        Automatically falls back to a triangular profile when the move is
 *        too short to reach cruise speed.
 *
 * @note  At least one of steps_x / steps_y must be non-zero.
 *
 * @param steps_x  Signed number of steps on the X axis (negative = reverse).
 * @param steps_y  Signed number of steps on the Y axis (negative = reverse).
 * @return MoveBlock_t  The fully initialised, ready-to-execute move block.
 */
MoveBlock_t StepGenerator_GenerateBlock(int32_t steps_x, int32_t steps_y);

/**
 * @brief Loads a move block and starts execution. Returns immediately.
 *        The caller retains ownership of the block and must ensure it
 *        remains valid until StepGenerator_IsBusy() returns false.
 *
 * @param block  Pointer to a MoveBlock_t from StepGenerator_GenerateBlock().
 * @return true   Move accepted and started.
 * @return false  Rejected — a move is already in progress.
 */
bool StepGenerator_StartMove(const MoveBlock_t* block);

/**
 * @brief Returns whether a move is currently in progress.
 *
 * @return true   A move is executing; do not call StepGenerator_StartMove().
 * @return false  The step generator is idle and ready for a new block.
 */
bool StepGenerator_IsBusy(void);

/* void StepGenerator_Abort(void); */ /* TODO: stop mid-move and clear state */

/**
 * @brief Step generator update routine — must be called from the timer ISR
 *        on every tick. Handles pulse clearing, DDA stepping, interval
 *        lookup, and move completion.
 *
 * @note  Not to be called from any other context.
 */
void StepGenerator_Update(void);

void StepGenerator_Abort(void);

#endif /* __STEP_GENERATOR_H__ */