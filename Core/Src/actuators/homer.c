#include "homer.h"

#include "interrupt.h"
#include "limit_switch.h"

#include <stdlib.h>

typedef enum {
  HS_UNINITIADED = 0,
  HS_COARSE = BIT(0),
  HS_BACKOFF = BIT(1),
  HS_FINE = BIT(2),
  HS_FINE_BACKOFF = BIT(3),
  HS_DONE = BIT(4),
  HS_IDLE = BIT(5)
} Homer_States_t;

/* TODO: homing watchdog ??

static uint32_t homing_watchdog = 0;
// in COARSE:
if (++homing_watchdog > MAX_HOMING_STEPS) {
    Interrupt_SetState(IS_ESTOP);
    return;
}
*/

static Homer_States_t homer_Phase = HS_UNINITIADED;
static uint32_t backoff_ticks_remaining = 0;

static Stepper_t* motor_x = NULL;
static Stepper_t* motor_y = NULL;
static uint32_t x_ticks = 0;
static uint32_t y_ticks = 0;

void Homer_Init(Stepper_t* mx, Stepper_t* my) {
  motor_x = mx;
  motor_y = my;
}

static void Homer_ClearSteps(void) {
  Stepper_ClearStep(motor_x);
  Stepper_ClearStep(motor_y);
}
static void Homer_MoveXCoarse(void) {
  if (x_ticks == 0) {
    Stepper_SetStep(motor_x);
    x_ticks = HOMING_COARSE_INTERVAL;
  } else {
    x_ticks--;
  }
}
static void Homer_MoveXFine(void) {
  if (x_ticks == 0) {
    Stepper_SetStep(motor_x);
    x_ticks = HOMING_FINE_INTERVAL;
  } else {
    x_ticks--;
  }
}
static void Homer_MoveYCoarse(void) {
  if (y_ticks == 0) {
    Stepper_SetStep(motor_y);
    y_ticks = HOMING_COARSE_INTERVAL;
  } else {
    y_ticks--;
  }
}

static void Homer_MoveYFine(void) {
  if (y_ticks == 0) {
    Stepper_SetStep(motor_y);
    y_ticks = HOMING_FINE_INTERVAL;
  } else {
    y_ticks--;
  }
}

static void Homer_MoveBackoff(void) {
  if (y_ticks == 0) {
    Stepper_SetStep(motor_y);
    y_ticks = HOMING_BACKOFF_INTERVAL;
  } else {
    y_ticks--;
  }
  if (x_ticks == 0) {
    Stepper_SetStep(motor_y);
    x_ticks = HOMING_BACKOFF_INTERVAL;
  } else {
    x_ticks--;
  }
}

void Homer_Update(void) {
  switch (homer_Phase) {
    case HS_COARSE:
      Homer_ClearSteps();
      switch (LimitSwitch_Activated() & (LIM_X_MIN | LIM_Y_MIN)) {
        case LIM_NO_LIM:
          Homer_MoveXCoarse();
          Homer_MoveYCoarse();
          return;
        case LIM_X_MIN:
          Homer_MoveYCoarse();
          return;
        case LIM_Y_MIN:
          Homer_MoveXCoarse();
          return;
        case (LIM_X_MIN | LIM_Y_MIN):
          homer_Phase = HS_BACKOFF;
          Stepper_SetDirection(motor_x, true);
          Stepper_SetDirection(motor_y, true);
          backoff_ticks_remaining = HOMING_BACKOFF_TICKS;
          Homer_MoveBackoff();
        default:
          return;
      }
      break;
    case HS_BACKOFF:
      Homer_ClearSteps();
      if (backoff_ticks_remaining == 0) {
        homer_Phase = HS_FINE;
        Stepper_SetDirection(motor_x, false);
        Stepper_SetDirection(motor_y, false);
        return;
      } else {
        backoff_ticks_remaining--;
        Homer_MoveBackoff();
      }
      break;
    case HS_FINE:
      Homer_ClearSteps();
      switch (LimitSwitch_Activated() & (LIM_X_MIN | LIM_Y_MIN)) {
        case LIM_NO_LIM:
          Homer_MoveXFine();
          Homer_MoveYFine();
          return;
        case LIM_X_MIN:
          Homer_MoveYFine();
          return;
        case LIM_Y_MIN:
          Homer_MoveXFine();
          return;
        case (LIM_X_MIN | LIM_Y_MIN):
          homer_Phase = HS_FINE_BACKOFF;
          Stepper_SetDirection(motor_x, true);
          Stepper_SetDirection(motor_y, true);
          backoff_ticks_remaining = HOMING_BACKOFF_TICKS;
          Homer_MoveXFine();
          Homer_MoveYFine();
        default:
          return;
      }
      break;
    case HS_FINE_BACKOFF:
      Homer_ClearSteps();
      if (backoff_ticks_remaining == 0) {
        homer_Phase = HS_DONE;
       return;
      } else {
        backoff_ticks_remaining--;
        Homer_MoveXFine();
        Homer_MoveYFine();
      }
      return;
    case HS_DONE:
      Homer_ClearSteps();
      homer_Phase = HS_FINE;
      Interrupt_SetState(IS_READY);
      break;
    default:
      break;
  }
}

void Homer_HomingStart(void) {
  homer_Phase = HS_COARSE;
  Stepper_SetDirection(motor_x, false);
  Stepper_SetDirection(motor_y, false);
  Interrupt_SetState(IS_HOMING);
}

bool Homer_IsRunning(void) { return !(homer_Phase & (HS_IDLE | HS_DONE)); }
