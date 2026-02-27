#ifndef __INIT_H__
#define __INIT_H__

#include "stepper.h"

// static 

static Stepper_t stepper_x;
static Stepper_t stepper_y;
static Stepper_t stepper_rot;
static MotionPlanner_t planner;
static EndEffector_t tool;
static StateMachine_t machine; // The actual memory for the state machine



void Sys_Init(void);




/*
void Sys_Init(void) {
    // 1. Init Steppers
    // 2. Init Planner with Stepper pointers
    Planner_Init(&planner, &x, &y, &rot); 
    
    // 3. Init Tool (Magnet, Piston, etc.)
    // 4. Finally, Link everything to the State Machine
    Machine_Init(&machine, &planner, &tool);
}

// Global accessor so main.c can call the run function
StateMachine_t* Get_Machine(void) {
    return &machine;}
*/

#endif