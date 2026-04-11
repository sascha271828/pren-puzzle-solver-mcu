#include "interrupt.h"

static InterruptState_t interruptState;

InterruptState_t Interrupt_GetState(void) { return interruptState; }
void Interrupt_Init(void) { interruptState = IS_INIT; }
void Interrupt_SetState(InterruptState_t is) { interruptState = is; }
