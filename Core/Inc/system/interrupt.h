#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

typedef enum {
  IS_ESTOP,
  IS_INIT,
  IS_READY,
  IS_HOMING,
  IS_RUNNING,
} InterruptState_t;

InterruptState_t Interrupt_GetState(void);
void Intterrupt_Init(void);
void Interrupt_SetState(InterruptState_t is);

#endif /* __INTERRUPT_H__ */