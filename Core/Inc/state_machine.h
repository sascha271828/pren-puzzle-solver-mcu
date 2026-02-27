#ifndef __STATE_MACHINE_H__
#define __STATE_MACHINE_H__

#include "motion_planer.h"
#include "end_effector.h"


typedef enum {
    STATE_ERROR = 1,
    STATE_IDLE,
    
    STATE_HOMING_START,
    STATE_LIMIT_HIT,
    STATE_HOMING_CHECK,

    STATE_REQUEST_DATA,
    STATE_WAIT_DATA,
    
    STATE_PROCESS_DATA,
    STATE_DECIDE_NEXT_ACTION,
    
    STATE_MOVE_XY,

    STATE_PICKUP,
    STATE_ROTATE,
    STATE_PLACE,
    
    STATE_SIGNAL_COMPLETION,
    STATE_MAX
}MachineState_e;





typedef struct {
    MachineState_e previous_state;
    MachineState_e current_state;

    MotionPlanner_t *planner;
    EndEffector_t   *tool;

    uint32_t state_timer_start; 
    uint32_t timeout_threshold;
    
    uint16_t current_move_index;
    uint16_t total_moves;
} StateMachine_t;




void Machine_Init(StateMachine_t* self, MotionPlanner_t* planner, EndEffector_t* tool);
void Machine_Run(StateMachine_t* self);

void Machine_Update(StateMachine_t* self);

 #endif