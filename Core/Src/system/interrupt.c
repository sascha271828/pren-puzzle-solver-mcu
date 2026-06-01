#include "interrupt.h"

/* ========================
 *   PRIVATE VARIABLES
 * ======================== */

volatile static InterruptState_t interruptState;

/* ========================
 *   PUBLIC API
 * ======================== */

InterruptState_t Interrupt_GetState(void) { return interruptState; }
void Interrupt_Init(void) { interruptState = IS_ESTOP; }
void Interrupt_SetState(InterruptState_t is) { interruptState = is; }
