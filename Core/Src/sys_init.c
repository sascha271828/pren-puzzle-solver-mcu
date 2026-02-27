#include "sys_init.h"
#include "actuator.h"
#include "end_effector.h"
#include "main.h"
#include "motion_planer.h"
#include "piston.h"
#include "state_machine.h"
#include "stepper.h"


// TODO: change to correct pins
// motor x-axis
static const StepperPin_t pins_stepper_x = {
#if CONFIG_FOR_ENABLE_DRIVER
    .enable = {
        .port = TEST_GPIO_GPIO_Port,
        .pin = TEST_GPIO_Pin
    },
#endif
#if CONFIG_FOR_NSLEEP_DRIVER
    .nsleep = {
        .port = TEST_GPIO_GPIO_Port,
        .pin = TEST_GPIO_Pin
    }, 
#endif
#if CONFIG_FOR_NFAULT_DRIVER
    .fault = {
        .port = TEST_GPIO_GPIO_Port,
        .pin = TEST_GPIO_Pin
    },  
#endif
    .step = {
        .port = TEST_GPIO_GPIO_Port,
        .pin = TEST_GPIO_Pin
    },  
    .dir = {
        .port = TEST_GPIO_GPIO_Port,
        .pin = TEST_GPIO_Pin
    },  
    .m0 = {
        .port = TEST_GPIO_GPIO_Port,
        .pin = TEST_GPIO_Pin
    },  
    .m1 = {
        .port = TEST_GPIO_GPIO_Port,
        .pin = TEST_GPIO_Pin
    }, 
    .limit_switch_min = {
        .port = TEST_GPIO_GPIO_Port,
        .pin = TEST_GPIO_Pin
    }, 
    .limit_switch_max = {
        .port = TEST_GPIO_GPIO_Port,
        .pin = TEST_GPIO_Pin
    }
};





// motor y-axis
static const StepperPin_t pins_stepper_y = {
#if CONFIG_FOR_ENABLE_DRIVER
    .enable = {
        .port = TEST_GPIO_GPIO_Port,
        .pin = TEST_GPIO_Pin
    },
#endif
#if CONFIG_FOR_NSLEEP_DRIVER
    .nsleep = {
        .port = TEST_GPIO_GPIO_Port,
        .pin = TEST_GPIO_Pin
    }, 
#endif
#if CONFIG_FOR_NFAULT_DRIVER
    .fault = {
        .port = TEST_GPIO_GPIO_Port,
        .pin = TEST_GPIO_Pin
    },  
#endif
    .step = {
        .port = TEST_GPIO_GPIO_Port,
        .pin = TEST_GPIO_Pin
    },  
    .dir = {
        .port = TEST_GPIO_GPIO_Port,
        .pin = TEST_GPIO_Pin
    },  
    .m0 = {
        .port = TEST_GPIO_GPIO_Port,
        .pin = TEST_GPIO_Pin
    },  
    .m1 = {
        .port = TEST_GPIO_GPIO_Port,
        .pin = TEST_GPIO_Pin
    }, 
    .limit_switch_min = {
        .port = TEST_GPIO_GPIO_Port,
        .pin = TEST_GPIO_Pin
    }, 
    .limit_switch_max = {
        .port = TEST_GPIO_GPIO_Port,
        .pin = TEST_GPIO_Pin
    }
};


// planer constants
static const float planner_steps_per_mm_x = 0;
static const float planner_steps_per_mm_y = 0;

static const float planner_max_velocity = 0;
static const float planner_max_acceleration = 0;



// motor rotation
static const StepperPin_t pins_stepper_rot = {
#if CONFIG_FOR_ENABLE_DRIVER
    .enable = {
        .port = TEST_GPIO_GPIO_Port,
        .pin = TEST_GPIO_Pin
    },
#endif
#if CONFIG_FOR_NSLEEP_DRIVER
    .nsleep = {
        .port = TEST_GPIO_GPIO_Port,
        .pin = TEST_GPIO_Pin
    }, 
#endif
#if CONFIG_FOR_NFAULT_DRIVER
    .fault = {
        .port = TEST_GPIO_GPIO_Port,
        .pin = TEST_GPIO_Pin
    },  
#endif
    .step = {
        .port = TEST_GPIO_GPIO_Port,
        .pin = TEST_GPIO_Pin
    },  
    .dir = {
        .port = TEST_GPIO_GPIO_Port,
        .pin = TEST_GPIO_Pin
    },  
    .m0 = {
        .port = TEST_GPIO_GPIO_Port,
        .pin = TEST_GPIO_Pin
    },  
    .m1 = {
        .port = TEST_GPIO_GPIO_Port,
        .pin = TEST_GPIO_Pin
    }, 
    .limit_switch_min = {
        .port = TEST_GPIO_GPIO_Port,
        .pin = TEST_GPIO_Pin
    }, 
    .limit_switch_max = {
        .port = TEST_GPIO_GPIO_Port,
        .pin = TEST_GPIO_Pin
    }
};



// piston
static const Piston_t piston = {
    .state = PISTON_STOP,
    .movement_time = 0,
    .piston_1_high = {
        .port = TEST_GPIO_GPIO_Port,
        .pin = TEST_GPIO_Pin
    },
    .piston_1_low = {
        .port = TEST_GPIO_GPIO_Port,
        .pin = TEST_GPIO_Pin
    },
        .piston_2_high = {
        .port = TEST_GPIO_GPIO_Port,
        .pin = TEST_GPIO_Pin
    },
        .piston_2_low = {
        .port = TEST_GPIO_GPIO_Port,
        .pin = TEST_GPIO_Pin
    },
    .piston_high_active = false,
    .piston_low_active = false
};


// magnet
static const BinaryActuator_t magnet = {
    .pin = {
        .port = TEST_GPIO_GPIO_Port,
        .pin = TEST_GPIO_Pin
    },
    .state = ACTUATOR_OFF,
    .active_low = false
};







void Sys_Init(void){
    Stepper_Init(&stepper_x,  pins_stepper_x);    
    Stepper_Init(&stepper_y,  pins_stepper_y);
    Stepper_Init(&stepper_rot,  pins_stepper_rot);

    
    // planer init

    Planner_Init(planner, stepper_x, stepper_y);
    
    Planer_SetMachineConstants(planner, 
        planner_steps_per_mm_x, planner_steps_per_mm_y );
    
    Planner_SetLimit(planner,
    planner_max_velocity,     planner_max_acceleration);


    Tool_Init(tool, magnet, piston, stepper_rot);

    Machine_Init(machine, planner, tool);






}





