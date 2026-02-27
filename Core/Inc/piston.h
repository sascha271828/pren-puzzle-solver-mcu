#ifndef __PISTON_H__
#define __PISTON_H__


#include "actuator.h"
#include "utils.h"

typedef enum {
    PISTON_STOP,
    PISTON_EXTENDING,
    PISTON_RETRACTING
} PistonState_e;


typedef struct{
    PistonState_e state;
    
    uint32_t movement_time;

    GPIO_Pin_t piston_1_high;
    GPIO_Pin_t piston_1_low;

    GPIO_Pin_t piston_2_high;
    GPIO_Pin_t piston_2_low;
    
    bool piston_high_active;
    bool piston_low_active;
    
}Piston_t;


void Piston_Set(Piston_t* self, PistonState_e action);

#endif