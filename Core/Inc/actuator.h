#ifndef __ACTUATOR_H__
#define __ACTUATOR_H__

#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"

#include "utils.h" 

typedef enum{
    ACTUATOR_OFF = 0,
    ACTUATOR_ON = 1
}ActuatorState_e;



typedef struct {
    GPIO_Pin_t pin;
    ActuatorState_e state;
    bool active_low;
} BinaryActuator_t;

void Actuator_Set(Actuator_t *self, ActuatorState_e state);
void Actuator_Toggle(Actuator_t *self);




#endif 