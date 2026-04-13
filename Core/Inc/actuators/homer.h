#ifndef __HOMER_H__
#define __HOMER_H__
/**
 *          |    _________
 *          |   /  mmmmm, \
 *          |   \doughnuts/   _
 *          |        O      _//\-\
 *          |         O    /      \
 *         _|_         o  /       |
 *        /   \         (.(.) /|\/
 *       |  0  |         (___    ,)
 *        \___/          /   \   \
 *            _          \o  /   |
 *          _( \_         _| _____\
 *         (___  \_______/\_/______\
 *         (___         /    /    \|
 *         (___________/     |____||
 *                    /      |    ||
 *                   /_______|    |_\
 *                   \      _|    | /
 *                    |    (_     \/
 *                    | \__  | | | |
 *                    |    \ |_|_|_|
 *                    |     |     |
 *                    |     |     |
 *                    |     |     |
 *                    |_____|_____|
 *                    |_____|_____|
 *                   /     /      |
 */

/**
 * @file homer.h
 * @brief Two-axis homing state machine for X and Y stepper axes.
 *
 * Implements a three-phase homing sequence driven by the shared timer ISR:
 *
 *  1. **Coarse** (HS_COARSE): Both axes move toward their minimum limit
 *     switches at a fast interval (HOMING_COARSE_INTERVAL).  Each axis
 *     stops independently as its switch triggers.
 *
 *  2. **Backoff** (HS_BACKOFF): Both axes reverse away from the switches
 *     for a fixed number of ticks (HOMING_BACKOFF_TICKS), clearing the
 *     switch contacts.
 *
 *  3. **Fine** (HS_FINE): Both axes re-approach the switches slowly
 *     (HOMING_FINE_INTERVAL) for a precise zero reference.  When both
 *     switches are active simultaneously the sequence is complete.
 * 
 *  4. **Fine-Bakcoff** (HS_FINE_BACKOFF): Both axes reverse away from
 *     the swtiches slowly (HOMING_FINE_INTERVAL) for a precise starting
 *     point away from the limit switches. 
 * 
 * On completion the system state transitions to IS_READY via
 * Interrupt_SetState().
 *
 * Compile-time parameters (define in sys_config.h):
 *  - HOMING_COARSE_INTERVAL  – ISR ticks between coarse steps
 *  - HOMING_FINE_INTERVAL    – ISR ticks between fine steps
 *  - HOMING_BACKOFF_INTERVAL – ISR ticks between backoff steps
 *  - HOMING_BACKOFF_TICKS    – total ticks spent in the backoff phase
 */

#include "stepper.h"
#include "sys_config.h"
#include "utils.h"

#include <stdbool.h>

/**
 * @brief Initialises the homing module with the two axis motor instances.
 *        Must be called once before Homer_HomingStart().
 *
 * @param mx  Pointer to the X-axis Stepper_t instance.
 * @param my  Pointer to the Y-axis Stepper_t instance.
 */
void Homer_Init(Stepper_t* mx, Stepper_t* my);

/**
 * @brief Advances the homing state machine by one ISR tick.
 *        Must be called from the shared timer ISR on every tick while
 *        the system is in IS_HOMING state.
 *
 * @note  Not to be called from any other context.
 */
void Homer_Update(void);

/**
 * @brief Starts the homing sequence from the beginning.
 *        Resets internal state, sets both axes to move toward their
 *        minimum limits, and transitions the system to IS_HOMING.
 *
 * @note  Homer_Init() must have been called before invoking this function.
 */
void Homer_HomingStart(void);

/**
 * @brief Returns whether the homing sequence has completed.
 *
 * @return true   Homing is done; system has transitioned to IS_READY.
 * @return false  Homing is still in progress or has not started.
 */
bool Homer_IsRunning(void);

#endif /* __HOMER_H__ */