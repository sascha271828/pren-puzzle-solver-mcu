#ifndef __SYS_CONFIG_MY_H__
#define __SYS_CONFIG_MY_H__

/* ============================================================================
 * sys_config.h — PuzzleSolver_MCU system configuration
 *
 * HOW TO USE
 * ----------
 * Edit only the values in the "SETTINGS" sections.
 * All derived constants are calculated automatically below.
 *
 * Axis (X/Y) : NEMA 17, 200 steps/rev, GT2 belt + 20-tooth pulley (40 mm circ)
 * Rotation   : NEMA 11, 200 steps/rev, direct drive or belt
 * ========================================================================== */

/* ============================================================================
 * TIMER
 * ========================================================================== */
#define TIMER_FREQ_HZ_ACTUATORS 120000UL /* TIM2 ISR frequency [Hz] */

/* ============================================================================
 * STEPPER DRIVER — microstepping pin encoding (DRV8886)
 *
 * M1 M0 → resolution
 *  0  0    full step
 *  0  1    1/2
 *  1  0    1/4
 *  1  1    1/16 (or higher depending on driver variant)
 * ========================================================================== */
#define STEPPER_MICRO_FULL 1UL
#define STEPPER_MICRO_1_2 2UL
#define STEPPER_MICRO_1_4 4UL
#define STEPPER_MICRO_1_16 16UL

/* ============================================================================
 * PISTON
 * ========================================================================== */
#define CONFIG_PISTON_HAS_LIMIT_SWITCH 0
#define CONFIG_PISTON_SEPARAT_PINS 0

#define CONFIG_PISTON_TIME_RETRACT_INIT 1000u   /* ms — retract from unknown */
#define CONFIG_PISTON_TIME_START_MOVE_MS 500u   /* ms — START → MOVE         */
#define CONFIG_PISTON_TIME_GRAB_MOVE_MS 500u    /* ms — MOVE ↔ GRAB          */
#define CONFIG_PISTON_TIME_MOVE_RELEASE_MS 500u /* ms — MOVE ↔ RELEASE */

/* ============================================================================
 * DRIVER FEATURE FLAGS
 * ========================================================================== */
#define CONFIG_FOR_NSLEEP_DRIVER 0 /* Set 1 if driver has nSLEEP pin */
#define CONFIG_STEPPER_NFAULT 0    /* Set 1 if driver has nFAULT pin */

/* ============================================================================
 * AXIS (X / Y) — NEMA 17
 * ========================================================================== */

/* --- Settings (edit these) ------------------------------------------------ */
#define CONFIG_AXIS_STEPS_PER_REV 200UL     /* Full steps per revolution   */
#define CONFIG_AXIS_MICRO STEPPER_MICRO_1_4 /* Microstepping divisor (1/4) */
                                            /* Options: 1, 2, 4, 16       */

/** Circumference of the drive pulley / shaft [mm].
 *  GT2 belt + 20-tooth pulley → 20 × 2 mm = 40 mm.
 *  Adjust if using a different pulley or leadscrew pitch. */
#define CONFIG_AXIS_CIRCUMFERENCE_MM 40UL

#define CONFIG_AXIS_MAX_SPEED_MM_S 200UL /* Cruise speed        [mm/s]  */
#define CONFIG_AXIS_ACCEL_MM_S2 3000UL   /* Acceleration        [mm/s²] */

/* --- Derived: steps/mm (kept as NUM/DEN fraction to avoid truncation) ----- */
#define AXIS_STEPS_PER_MM_NUM (CONFIG_AXIS_MICRO * CONFIG_AXIS_STEPS_PER_REV)
#define AXIS_STEPS_PER_MM_DEN (CONFIG_AXIS_CIRCUMFERENCE_MM)
/* Effective steps/mm = AXIS_STEPS_PER_MM_NUM / AXIS_STEPS_PER_MM_DEN
 * e.g. 4×200/40 = 20 steps/mm  */

/* --- Derived: velocity and acceleration in steps/s and steps/s² ----------- */
#define AXIS_MAX_V_STEPS \
  (CONFIG_AXIS_MAX_SPEED_MM_S * AXIS_STEPS_PER_MM_NUM / AXIS_STEPS_PER_MM_DEN)

#define AXIS_ACCEL_STEPS_S2 \
  (CONFIG_AXIS_ACCEL_MM_S2 * AXIS_STEPS_PER_MM_NUM / AXIS_STEPS_PER_MM_DEN)

/* --- Derived: cruise timer-tick interval -----------------------------------
 */
/** Number of ISR ticks between steps at cruise speed.
 *  cruise_interval = TIMER_FREQ / v_steps  */
#define AXIS_CRUISE_INTERVAL (TIMER_FREQ_HZ_ACTUATORS / AXIS_MAX_V_STEPS)

/* --- Derived: acceleration ramp table length -------------------------------
 */
/** Theoretical minimum accel steps from Austin formula: v² / (2·a).
 *  The discrete approximation needs ~2.15× more steps to actually converge
 *  to cruise speed, so we apply that factor here.
 *  The result is used as the interval-table array size and the ramp length. */
#define AXIS_ACCEL_STEPS_IDEAL                     \
  (215UL * (AXIS_MAX_V_STEPS * AXIS_MAX_V_STEPS) / \
   (2UL * AXIS_ACCEL_STEPS_S2 * 100UL))

/* ============================================================================
 * ROTATION — NEMA 11
 * ========================================================================== */

/* --- Settings (edit these) ------------------------------------------------ */
#define CONFIG_ROT_STEPS_PER_REV 200UL      /* Full steps per revolution   */
#define CONFIG_ROT_MICRO STEPPER_MICRO_1_16 /* Microstepping divisor (1/16)*/
                                            /* Options: 1, 2, 4, 16       */

/** Circumference of the rotating stage / belt [mm].
 *  Measure the actual travel distance for one full shaft revolution. */
#define CONFIG_ROT_CIRCUMFERENCE_MM 100UL

#define CONFIG_ROT_MAX_SPEED_MM_S 50UL /* Cruise speed        [mm/s]  */
#define CONFIG_ROT_ACCEL_MM_S2 100UL   /* Acceleration        [mm/s²] */

/* --- Derived: steps/mm ---------------------------------------------------- */
#define ROT_STEPS_PER_MM_NUM (CONFIG_ROT_MICRO * CONFIG_ROT_STEPS_PER_REV)
#define ROT_STEPS_PER_MM_DEN (CONFIG_ROT_CIRCUMFERENCE_MM)
/* Effective steps/mm = ROT_STEPS_PER_MM_NUM / ROT_STEPS_PER_MM_DEN
 * e.g. 16×200/100 = 32 steps/mm  */

/* --- Derived: velocity and acceleration in steps/s and steps/s² ----------- */
#define ROT_MAX_V_STEPS \
  (CONFIG_ROT_MAX_SPEED_MM_S * ROT_STEPS_PER_MM_NUM / ROT_STEPS_PER_MM_DEN)

#define ROT_ACCEL_STEPS_S2 \
  (CONFIG_ROT_ACCEL_MM_S2 * ROT_STEPS_PER_MM_NUM / ROT_STEPS_PER_MM_DEN)

/* --- Derived: cruise interval ----------------------------------------------
 */
#define ROT_CRUISE_INTERVAL (TIMER_FREQ_HZ_ACTUATORS / ROT_MAX_V_STEPS)

/* --- Derived: ramp table length ------------------------------------------- */
#define ROT_ACCEL_STEPS_IDEAL                    \
  (215UL * (ROT_MAX_V_STEPS * ROT_MAX_V_STEPS) / \
   (2UL * ROT_ACCEL_STEPS_S2 * 100UL))

/* ============================================================================
 * COMPILE-TIME SANITY CHECKS
 * ========================================================================== */

/* Cruise interval must be reachable within a single 32-bit tick counter */
#if AXIS_CRUISE_INTERVAL == 0
#error \
    "AXIS_CRUISE_INTERVAL is 0 — MAX_SPEED too high or CIRCUMFERENCE too small"
#endif
#if ROT_CRUISE_INTERVAL == 0
#error \
    "ROT_CRUISE_INTERVAL is 0 — ROT_MAX_SPEED too high or CIRCUMFERENCE too small"
#endif

/* Ramp table must have at least a few entries */
#if AXIS_ACCEL_STEPS_IDEAL < 4
#error "AXIS_ACCEL_STEPS_IDEAL < 4 — increase MAX_SPEED or decrease ACCEL"
#endif
#if ROT_ACCEL_STEPS_IDEAL < 4
#error \
    "ROT_ACCEL_STEPS_IDEAL < 4 — increase ROT_MAX_SPEED or decrease ROT_ACCEL"
#endif

#endif /* __SYS_CONFIG_MY_H__ */