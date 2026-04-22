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

#define BIT(n) (1u << (n))

/* ── Run-mode selection ──────────────────────────────────────────────────
 * Set exactly one of these to select what App_Run() executes.
 * RUN_MODE_TEST_CLI : interactive UART command interface (development)
 * RUN_MODE_APP      : production loop driven by CommandDispatcher
 * ─────────────────────────────────────────────────────────────────────── */
#define RUN_MODE_TEST_CLI 0
#define RUN_MODE_APP 1

#define RUN_MODE RUN_MODE_TEST_CLI

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

#define CONFIG_PISTON_TIME_RETRACT_INIT_MS \
  800u /* ms — retract from unknown      \
        */

#define CONFIG_PISTON_PWM_ENUMERATER 3u
#define CONFIG_PISTON_PWM_DIVISOR 12u

/* absolute time from start position [ms]*/
#define PISTON_OFFSET_START_MS 0u
#define PISTON_OFFSET_MOVE_MS 100u
#define PISTON_OFFSET_GRAB_MS 400u
#define PISTON_OFFSET_RELEASE_MS 500u

/* Derived tick counts — do not edit.
 * Result fits in int32_t: max = 120000 * 2000 / 1000 = 240000 << INT32_MAX  */
#define PISTON_MS_TO_TICKS(ms) \
  ((int32_t)((TIMER_FREQ_HZ_ACTUATORS) * (ms) / 1000UL))

#define CONFIG_PISTON_TICKS_RETRACT_INIT \
  PISTON_MS_TO_TICKS(CONFIG_PISTON_TIME_RETRACT_INIT_MS)

#define CONFIG_PISTON_TICKS_START_MOVE \
  PISTON_MS_TO_TICKS(CONFIG_PISTON_TIME_START_MOVE_MS)

#define CONFIG_PISTON_TICKS_MOVE_GRAB \
  PISTON_MS_TO_TICKS(CONFIG_PISTON_TIME_MOVE_GRAB_MS)

#define CONFIG_PISTON_TICKS_MOVE_RELEASE \
  PISTON_MS_TO_TICKS(CONFIG_PISTON_TIME_MOVE_RELEASE_MS)

/* Multi-hop shortcuts (START→GRAB skips through MOVE implicitly) */
#define CONFIG_PISTON_TICKS_START_GRAB                    \
  PISTON_MS_TO_TICKS((CONFIG_PISTON_TIME_START_MOVE_MS) + \
                     (CONFIG_PISTON_TIME_MOVE_GRAB_MS))

#define CONFIG_PISTON_TICKS_START_RELEASE                 \
  PISTON_MS_TO_TICKS((CONFIG_PISTON_TIME_START_MOVE_MS) + \
                     (CONFIG_PISTON_TIME_MOVE_RELEASE_MS))

#define CONFIG_PISTON_TICKS_GRAB_RELEASE                 \
  PISTON_MS_TO_TICKS((CONFIG_PISTON_TIME_MOVE_GRAB_MS) + \
                     (CONFIG_PISTON_TIME_MOVE_RELEASE_MS))

/* ============================================================================
 * DRIVER FEATURE FLAGS
 * ========================================================================== */
#define CONFIG_FOR_NSLEEP_DRIVER 0 /* Set 1 if driver has nSLEEP pin */
#define CONFIG_STEPPER_NFAULT 0    /* Set 1 if driver has nFAULT pin */

/* ============================================================================
 * AXIS (X / Y) — NEMA 17
 * ========================================================================== */

/* --- Settings (edit these) ------------------------------------------------ */
#define CONFIG_AXIS_STEPS_PER_REV 200UL /* Full steps per revolution   */
#define CONFIG_AXIS_MICRO STEPPER_MICRO_1_16
/* Options: 1, 2, 4, 16       */

/** Circumference of the drive pulley / shaft [mm].
 *  GT2 belt + 20-tooth pulley → 20 × 2 mm = 40 mm.
 *  Adjust if using a different pulley or leadscrew pitch. */
#define CONFIG_AXIS_CIRCUMFERENCE_MM 40UL

#define CONFIG_AXIS_MAX_SPEED_MM_S 250UL /* Cruise speed        [mm/s]  */
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

/* ============================================================================
 * HOMING
 * ========================================================================== */

/* Speeds as fraction of cruise speed */
#define CONFIG_HOMING_COARSE_SPEED_MM_S 40UL  /* fast search          */
#define CONFIG_HOMING_FINE_SPEED_MM_S 10UL    /* slow precise touch   */
#define CONFIG_HOMING_BACKOFF_SPEED_MM_S 20UL /* retreat              */
#define CONFIG_HOMING_BACKOFF_DIST_MM 5UL     /* retreat distance     */

/* Derived: ticks between steps (= ISR interval) */
#define HOMING_COARSE_INTERVAL                       \
  (TIMER_FREQ_HZ_ACTUATORS * AXIS_STEPS_PER_MM_DEN / \
   (CONFIG_HOMING_COARSE_SPEED_MM_S * AXIS_STEPS_PER_MM_NUM))

#define HOMING_FINE_INTERVAL                         \
  (TIMER_FREQ_HZ_ACTUATORS * AXIS_STEPS_PER_MM_DEN / \
   (CONFIG_HOMING_FINE_SPEED_MM_S * AXIS_STEPS_PER_MM_NUM))

#define HOMING_BACKOFF_INTERVAL                      \
  (TIMER_FREQ_HZ_ACTUATORS * AXIS_STEPS_PER_MM_DEN / \
   (CONFIG_HOMING_BACKOFF_SPEED_MM_S * AXIS_STEPS_PER_MM_NUM))

/* Derived: total ticks for backoff distance */
#define HOMING_BACKOFF_TICKS                               \
  (CONFIG_HOMING_BACKOFF_DIST_MM * AXIS_STEPS_PER_MM_NUM / \
   AXIS_STEPS_PER_MM_DEN * HOMING_BACKOFF_INTERVAL)

#endif /* __SYS_CONFIG_MY_H__ */