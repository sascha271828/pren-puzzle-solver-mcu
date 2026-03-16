#ifndef __SYS_CONFIG_H__
#define __SYS_CONFIG_H__

/* Stepper */
#define CONFIG_FOR_NFAULT_DRIVER    (0)     /* wheter the nFAULT detection of the stepper drivers is implemented */

#define CONFIG_FOR_ENABLE_DRIVER    (0)     /* wheter the ENABLE pins for the stepper drivers are controlled through software */
#define CONFIG_FOR_NSLEEP_DRIVER    (0)     /* wheter the nSLEEP pins for the stepper drivers are controlled through software */
#define CONFIG_DEFAULT_STEPPER_SPEED (100u)
#define CONFIG_DEFAULT_STEPPER_ACCEL (100u)

/* TODO set value to something reasonable */




/* piston */
#define CONFIG_PISTON_HAS_LIMIT_SWITCH (1)  /* whetere the piston has a limit switch */


#endif /* __SYS_CONFIG_H__ */