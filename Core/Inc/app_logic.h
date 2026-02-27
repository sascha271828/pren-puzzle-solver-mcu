#ifndef __APP_LOGIC__
#define __APP_LOGIC__



typedef struct {
    MachineState_e state;
    void (*entry_action)(void);   // Runs once when entering the state
    void (*run_action)(void);     // Runs repeatedly while in the state
    void (*exit_action)(void);    // Runs once when leaving
} StateTransition_t;

/*
const StateTransition_t StateTable[] = {
    {STATE_IDLE,          Idle_Entry,  Idle_Run,  Idle_Exit},
    {STATE_HOMING_START,  Home_Entry,  Home_Run,  Home_Exit},
    {STATE_MOVE_XY,       Move_Entry,  Move_Run,  Move_Exit},
    {STATE_ERROR,         Error_Entry, Error_Run, NULL}
};*/







#endif