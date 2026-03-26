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


/* Machine Constants */

/* stepper */
#define MAX_ACCEL_STEPS 256
#define CONFIG_STEPS_PER_MM_X (80.0f)
#define CONFIG_STEPS_PER_MM_Y (80.0f)
#define CONFIG_MAX_SPEED_AXIS (100.0f)
#define CONFIG_ACCEL_AXIS_MM_S2 (500.0f)
#define TIMER_FREQ_HZ_STEP (120000000UL) /* TODO take from STM32 */

/* rotation */
#define MAX_ACCEL_STEPS_ROT 256
#define CONFIG_STEPS_PER_01_DEGREE (80.0f)
#define CONFIG_MAX_SPEED_ROT (100.0f)
#define CONFIG_ACCEL_AXIS_ROT (500.0f)

/* piston */
#define CONFIG_PISTON_HAS_LIMIT_SWITCH \
  (0) /* whetere the piston has a limit switch */
#define CONFIG_PISTON_SEPARAT_PINS (0)
#define CONFIG_PISTON_TIME_START_MOVE_MS (500u)
#define CONFIG_PISTON_TIME_GRAB_MOVE_MS (500u)
#define CONFIG_PISTON_TIME_MOVE_RELEASE_MS (500u)
#define CONFIG_PISTON_TIME_RETRACT_INIT (1000u)

#endif /* __SYS_CONFIG_H__ */