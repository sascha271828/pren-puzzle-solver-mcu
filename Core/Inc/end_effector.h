#ifndef __END_EFFECTOR_H__
#define __END_EFFECTOR_H__

#include "actuator.h"
#include "piston.h"
#include "stepper.h"

typedef enum {
    TOOL_IDLE,
    TOOL_BUSY,
    TOOL_ERROR
} ToolState_t;


typedef struct {
    volatile ToolState_t state;
    BinaryActuator_t* magnet;
    Piston_t* piston;
    Stepper_t* rotator;

    uint32_t last_action_time;    
} EndEffector_t;

void Tool_PickUpSequence(EndEffector_t* self);
void Tool_RotatePiece(EndEffector_t* self, int16_t degrees);
void Tool_PlaceSequence(EndEffector_t* self);





#endif