#include "state_machine.h"


void Machine_Init(StateMachine_t* self, MotionPlanner_t* planner, EndEffector_t* tool){
    self->previous_state = STATE_IDLE;
    self->current_state = STATE_IDLE;
    self->planner = planner;
    self->tool = tool;
    
    // TODO?
    self->state_timer_start = 0;
    self->timeout_threshold = 0;

    self->current_move_index = 0;
    self->total_moves = 0;

}



