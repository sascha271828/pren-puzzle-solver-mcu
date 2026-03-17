#ifndef __SYS_CONFIG_MY_H__
#define __SYS_CONFIG_MY_H__

/* Stepper */


#define CONFIG_FOR_ENABLE_DRIVER                                               \
  (0) /* wheter the ENABLE pins for the stepper drivers are controlled through \
         software */
#define CONFIG_FOR_NSLEEP_DRIVER                                               \
  (0) /* wheter the nSLEEP pins for the stepper drivers are controlled through \
         software */
#define CONFIG_STEPPER_MICRO (0)
#define CONFIG_STEPPER_NFAULT (0)
#define CONFIG_DEFAULT_STEPPER_SPEED (100u)
#define CONFIG_DEFAULT_STEPPER_ACCEL (100u)

/* TODO set value to something reasonable */

/* Machine Constants */

#define CONFIG_CONSTANT_X_STEPS_PER_MM (0.0f)
#define CONFIG_CONSTANT_Y_STEPS_PER_MM (0.0f)

#define CONFIG_MAX_VELOCITY_MM_S (0.0f)
#define CONFIG_MAX_ACCELERATION_MM_SS (0.0f)

/* piston */
#define CONFIG_PISTON_HAS_LIMIT_SWITCH \
  (0) /* whetere the piston has a limit switch */
#define CONFIG_PISTON_SEPARAT_PINS (0)
#define CONFIG_PISTON_TIME_START_MOVE_MS (500u)
#define CONFIG_PISTON_TIME_GRAB_MOVE_MS (500u)
#define CONFIG_PISTON_TIME_MOVE_RELEASE_MS (500u)
#define CONFIG_PISTON_TIME_RETRACT_INIT (1000u)

#endif /* __SYS_CONFIG_H__ */