#ifndef __STEPPER_H__
#define __STEPPER_H__

/**
 * @file stepper.h
 * @brief Low-level stepper motor driver for DRV8886-based axes.
 *
 * Provides a thin HAL wrapper around a single stepper motor channel.
 * Each motor is represented by a Stepper_t struct which holds its GPIO
 * pin descriptors and runtime state.  All GPIO operations are performed
 * directly via HAL_GPIO_WritePin(); no timer or interrupt logic lives here.
 *
 * Intended use:
 *  - Call Stepper_Init() once per motor during system startup.
 *  - Use Stepper_SetDirection() before starting a move sequence.
 *  - Pulse generation is split across two ISR calls: Stepper_SetStep()
 *    asserts the STEP pin and Stepper_ClearStep() de-asserts it one tick
 *    later. The DRV8886 requires a minimum pulse width of 1 µs; at
 *    120 kHz ISR rate one tick ≈ 8.3 µs, so this is always satisfied.
 *
 * Microstepping truth table (DRV8886 M1/M0):
 * @verbatim
 *   M1  M0  | Mode
 *   --------+---------
 *    0   0  | Full step
 *    0   1  | 1/16
 *    1   0  | 1/2
 *    1   1  | 1/4
 * @endverbatim
 *
 * Hardware notes:
 *  - M0/M1 pins: Push-Pull, no pull-up/down, initial state Low.
 *  - ENABLE pin: active-high for DRV8886 (nSLEEP variant differs).
 *  - The DRV8886 indexer has no holding torque before the first step;
 *    account for this during initialisation if positional accuracy matters.
 *
 * Compile-time options (define in sys_config.h):
 *  - CONFIG_FOR_NSLEEP_DRIVER  – include nSLEEP pin in StepperPin_t
 *  - CONFIG_STEPPER_NFAULT     – include nFAULT pin and fault tracking
 */

#include "sys_config.h"
#include "utils.h"

#include <stdbool.h>

/**
 * @brief Microstepping resolution, mapped to DRV8886 M1/M0 pin states.
 *        Use StepperMicro_e values with Stepper_SetMicrostep().
 */
typedef enum {
  STEP_FULL = STEPPER_MICRO_FULL, /**< Full step  (M1=0, M0=0)             */
  STEP_1_2  = STEPPER_MICRO_1_2,  /**< Half step  (M1=1, M0=0)             */
  STEP_1_4  = STEPPER_MICRO_1_4,  /**< 1/4 step   (M1=1, M0=1)             */
  STEP_1_16 = STEPPER_MICRO_1_16, /**< 1/16 step  (M1=0, M0=1)             */
} StepperMicro_e;

/**
 * @brief High-level operating state of a stepper axis.
 *        Managed externally; the driver itself does not enforce transitions.
 */
typedef enum {
  STEPPER_ERROR  = 1, /**< Fault condition detected                         */
  STEPPER_IDLE,       /**< Motor is enabled but not moving                  */
  STEPPER_HOMING,     /**< Homing sequence in progress                      */
  STEPPER_MOVING,     /**< Motion sequence in progress                      */
} StepperState_e;

/**
 * @brief GPIO pin assignments for one stepper motor channel.
 *
 * All pins must be configured as Push-Pull outputs before Stepper_Init()
 * is called.  Optional pins are conditionally compiled via sys_config.h.
 */
typedef struct {
  GPIO_Pin_t enable; /**< ENABLE output (active-high)                       */
#if CONFIG_FOR_NSLEEP_DRIVER
  GPIO_Pin_t nsleep; /**< nSLEEP output (active-low sleep; optional)        */
#endif
#if CONFIG_STEPPER_NFAULT
  GPIO_Pin_t fault;  /**< nFAULT input  (active-low fault; optional)        */
#endif
  GPIO_Pin_t step;   /**< STEP output — rising edge advances indexer        */
  GPIO_Pin_t dir;    /**< DIR  output — level selects rotation direction     */
  GPIO_Pin_t m0;     /**< M0 microstepping select output                    */
  GPIO_Pin_t m1;     /**< M1 microstepping select output                    */
} StepperPin_t;

/**
 * @brief Runtime state for a single stepper motor instance.
 *
 * Embed one of these per motor axis.  All volatile fields are written from
 * the timer ISR and read from the main context.
 */
typedef struct {
  StepperPin_t pins;                   /**< GPIO descriptors for this axis  */

  volatile StepperState_e state;       /**< Current high-level axis state   */
  volatile int32_t        current_position; /**< Signed step count from home*/
  volatile StepperMicro_e current_micro;    /**< Active microstepping mode  */
  volatile bool           is_enabled;  /**< true when ENABLE pin is asserted*/
  volatile bool           direction;   /**< Current DIR pin level           */
  volatile bool           is_homed;    /**< true after successful homing    */
  volatile bool           pulse_active;/**< true while STEP pin is high     */
#if CONFIG_STEPPER_NFAULT
  volatile bool           has_fault;   /**< true when nFAULT is asserted   */
#endif
} Stepper_t;

/**
 * @brief Initialises a stepper motor instance and configures its GPIO pins.
 *        Applies the requested microstepping mode via Stepper_SetMicrostep().
 *        Sets initial state to STEPPER_IDLE with position 0.
 *
 * @param self    Pointer to the Stepper_t instance to initialise.
 * @param pins    GPIO pin assignments for this motor.
 * @param micro   Initial microstepping resolution.
 * @param enable  If true, assert the ENABLE pin immediately after init.
 */
void Stepper_Init(Stepper_t* self,
                  StepperPin_t pins,
                  StepperMicro_e micro,
                  bool enable);

/**
 * @brief Sets the direction of rotation and updates the DIR pin.
 *        Must be called before starting a new move in the opposite direction.
 *
 * @param self  Pointer to the Stepper_t instance.
 * @param dir   true = positive direction, false = negative direction.
 */
void Stepper_SetDirection(Stepper_t* self, bool dir);

/**
 * @brief Asserts the STEP pin (rising edge) and increments the position
 *        counter according to the current direction.
 *        Sets pulse_active = true.
 *
 * @param self  Pointer to the Stepper_t instance.
 */
void Stepper_SetStep(Stepper_t* self);

/**
 * @brief De-asserts the STEP pin and clears pulse_active.
 *        Must be called one ISR tick after Stepper_SetStep() to complete
 *        the step pulse.
 *
 * @param self  Pointer to the Stepper_t instance.
 */
void Stepper_ClearStep(Stepper_t* self);

/**
 * @brief Configures the M0/M1 pins for the requested microstepping mode.
 *
 * @param self  Pointer to the Stepper_t instance.
 * @param res   Desired microstepping resolution.
 */
void Stepper_SetMicrostep(Stepper_t* self, StepperMicro_e res);

/**
 * @brief Asserts or de-asserts the ENABLE pin.
 *
 * @param self    Pointer to the Stepper_t instance.
 * @param enable  true = enable driver outputs; false = disable (coast).
 */
void Stepper_Enable(Stepper_t* self, bool enable);

#endif /* __STEPPER_H__ */