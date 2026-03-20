#ifndef __COMMAND_DISPATCHER_H__
#define __COMMAND_DISPATCHER_H__

#include "puzzle.pb.h"
#include "uart_receiver.h"

#include <stdbool.h>

typedef struct {
  UartReceiver_t *uart;
  PuzzleCommand last_command;
  volatile bool command_pending;
} CommandDispatcher_t;

void CommandDispatcher_Init(CommandDispatcher_t *self, UartReceiver_t *uart);

/* Call from main loop — checks for new frames, decodes, sets pending flag */
void CommandDispatcher_Poll(CommandDispatcher_t *self);

/* Check if a decoded command is waiting */
bool CommandDispatcher_HasCommand(const CommandDispatcher_t *self);

/* Get the pending command and clear the flag */
PuzzleCommand *CommandDispatcher_GetCommand(CommandDispatcher_t *self);

/* Send an Ack back to the host */
void CommandDispatcher_SendAck(CommandDispatcher_t *self, Status status,
                               uint32_t piece_id);

#endif /* __COMMAND_DISPATCHER_H__ */
