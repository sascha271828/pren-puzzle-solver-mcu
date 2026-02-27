#ifndef __STATE_MACHINE_H__
#define __STATE_MACHINE_H__

#include "motion_planer.h"
#include "stepper.h"



enum machine_states_e{
    IDLE = 1,
    INITIALIZING,
    WAIT_START,
    WAIT_DATA,
    MOVE_XY,
    MOVE_Z,
    HOMING,
    GRAB,
    DROP,
    ERROR,
    SIGNAL_COMPLETION
};


typedef struct{
    


}state_machine_t;






 #endif