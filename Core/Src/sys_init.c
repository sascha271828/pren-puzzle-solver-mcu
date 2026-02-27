#include "sys_init.h"
#include "stepper.h"

private const StepperPin_t pins_stepper_x = {
#if CONFIG_FOR_ENABLE_DRIVER
    GPIO_Pin_t enable = {
        port* = GPIO,
        pin = 0,
    }
#endif
#if CONFIG_FOR_NSLEEP_DRIVER
    GPIO_Pin_t nsleep; 
#endif
#if CONFIG_FOR_NFAULT_DRIVER
    GPIO_Pin_t fault; 
#endif
    GPIO_Pin_t step; 
    GPIO_Pin_t dir; 
    GPIO_Pin_t m0; 
    GPIO_Pin_t m1;
    GPIO_Pin_t limit_switch_min;
    GPIO_Pin_t limit_switch_max;

};

typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
} GPIO_Pin_t;


void Sys_Init(void){
    Stepper_Init(&stepper_x,  pins_stepper_x);







}





