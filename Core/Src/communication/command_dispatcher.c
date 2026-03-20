#include "command_dispatcher.h"

#include <pb_decode.h>
#include <pb_encode.h>

#include <string.h>

void CommandDispatcher_Init(CommandDispatcher_t *self, UartReceiver_t *uart) {
  memset(self, 0, sizeof(*self));
  self->uart = uart;
}

void CommandDispatcher_Poll(CommandDispatcher_t *self) {
  if (!UartReceiver_IsFrameReady(self->uart)) return;
  if (self->command_pending) return;

  uint8_t raw[PuzzleCommand_size];
  size_t len = UartReceiver_GetPayload(self->uart, raw, sizeof(raw));
  if (len == 0) return;

  pb_istream_t stream = pb_istream_from_buffer(raw, len);
  PuzzleCommand cmd = PuzzleCommand_init_zero;

  if (pb_decode(&stream, PuzzleCommand_fields, &cmd)) {
    self->last_command = cmd;
    self->command_pending = true;
    CommandDispatcher_SendAck(self, Status_STATUS_OK, 0);
  } else {
    CommandDispatcher_SendAck(self, Status_STATUS_ERROR, 0);
  }
}

bool CommandDispatcher_HasCommand(const CommandDispatcher_t *self) {
  return self->command_pending;
}

PuzzleCommand *CommandDispatcher_GetCommand(CommandDispatcher_t *self) {
  self->command_pending = false;
  return &self->last_command;
}

void CommandDispatcher_SendAck(CommandDispatcher_t *self, Status status,
                               uint32_t piece_id) {
  Ack ack = Ack_init_zero;
  ack.status = status;
  ack.piece_id = piece_id;

  uint8_t buf[Ack_size];
  pb_ostream_t stream = pb_ostream_from_buffer(buf, sizeof(buf));

  if (pb_encode(&stream, Ack_fields, &ack)) {
    UartReceiver_SendFrame(self->uart, buf, stream.bytes_written);
  }
}
