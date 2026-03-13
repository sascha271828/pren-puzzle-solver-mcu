#ifndef __END_EFFECTOR_H__
#define __END_EFFECTOR_H__
/*
#include "sys_config.h"

#include "actuator.h"
#include "piston.h"
#include "stepper.h"

typedef enum {
    TOOL_IDLE,
    TOOL_BUSY,
    TOOL_ERROR
} ToolState_t;

//TODO translation rotation

typedef struct {
    volatile ToolState_t state;
    BinaryActuator_t* magnet;
    Piston_t* piston;
    Stepper_t* rotator;

    volatile uint32_t last_action_time;    
} EndEffector_t;


void Tool_Init(EndEffector_t* self, BinaryActuator_t* magnet,
    Piston_t* piston, Stepper_t* rotator);


void Tool_PickUpSequence(EndEffector_t* self);
void Tool_RotatePiece(EndEffector_t* self, int16_t degrees);
void Tool_PlaceSequence(EndEffector_t* self);
*/




#endif