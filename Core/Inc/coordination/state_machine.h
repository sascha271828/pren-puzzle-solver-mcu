#ifndef __STATE_MACHINE_H__
#define __STATE_MACHINE_H__

#include "command_dispatcher.h"
#include "piston.h"
#include "magnet.h"
#include "rotator.h"

void StateMachine_Init(CommandDispatcher_t* dispatcher, Piston_t* piston, Magnet_t* magnet, Rotator_t* rotator);

void StateMachine_Update(void);

#endif /* __STATE_MACHINE_H__ */