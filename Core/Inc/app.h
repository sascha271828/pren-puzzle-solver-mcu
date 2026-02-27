#ifndef __APP_H__
#define __APP_H__


#include "state_machine.h"
#include "app_logic.h"







void App_Run(void);


/*
void App_Run(void) {
    // Get the keys to the machine
    SystemContext_t* sys = Sys_GetContext();

    // Use them in your State Machine
    switch (current_state) {
        case STATE_MOVE_XY:
            Planner_MoveTo(sys->planner, 100.0, 50.0);
            break;
            
        case STATE_GRAB:
            Tool_Grab(sys->tool);
            break;
    }
}

*/



#endif