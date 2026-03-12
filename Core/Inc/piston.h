#ifndef __PISTON_H__
#define __PISTON_H__

#include "utils.h"
#include "sys_config.h"

#include "actuator.h"


typedef enum {
    PISTON_STOP,
    PISTON_EXTENDING,
    PISTON_RETRACTING,
} PistonState_e;


typedef struct{
    volatile PistonState_e state;
    
    uint32_t movement_time;

    GPIO_Pin_t piston_1_high;
    GPIO_Pin_t piston_1_low;

    GPIO_Pin_t piston_2_high;
    GPIO_Pin_t piston_2_low;
    
#if CONFIG_PISTON_HAS_LIMIT_SWITCH
    GPIO_Pin_t limit_switch_extended;
    GPIO_Pin_t limit_switch_contracted;
#endif    
bool piston_high_active;
bool piston_low_active;

}Piston_t;


void Piston_Set(Piston_t* self, PistonState_e action);

#endif  /* end of __PISONT_H__ */