#ifndef __COMMAND_DISPATCHER_H__
#define __COMMAND_DISPATCHER_H__

/**
 * @file command_dispatcher.h
 * @brief Protobuf command dispatcher over UART.
 *
 * Sits on top of UartReceiver and decodes incoming length-prefixed protobuf
 * frames (PuzzleCommand) using nanopb. Successfully decoded commands are
 * stored internally and a STATUS_OK Ack is sent to the host; malformed
 * frames get a STATUS_ERROR Ack.
 *
 * Typical usage:
 * @code
 *   CommandDispatcher_Init(&dispatcher, &uart_receiver);
 *
 *   // In main loop:
 *   CommandDispatcher_Poll(&dispatcher);
 *   if (CommandDispatcher_HasCommand(&dispatcher)) {
 *     PuzzleCommand *cmd = CommandDispatcher_GetCommand(&dispatcher);
 *     // process cmd ...
 *   }
 * @endcode
 */

#include "puzzle.pb.h"
#include "uart_receiver.h"

#include <stdbool.h>

/**
 * @brief Internal state for one dispatcher instance.
 *        One UartReceiver_t instance must outlive this struct.
 */
typedef struct {
  UartReceiver_t *uart;          /**< Underlying byte-stream receiver        */
  PuzzleCommand last_command;    /**< Most recently decoded command           */
  volatile bool command_pending; /**< True while a decoded command awaits     */
} CommandDispatcher_t;

/**
 * @brief Initialises the dispatcher and clears all state.
 *
 * @param self  Dispatcher instance to initialise.
 * @param uart  Pointer to an already-initialised UartReceiver_t.
 */
void CommandDispatcher_Init(CommandDispatcher_t *self, UartReceiver_t *uart);

/**
 * @brief Polls the UART receiver for a new frame and decodes it.
 *        Must be called from the main loop. No-op if no frame is available
 *        or if a previously decoded command has not yet been consumed.
 *
 * @param self  Dispatcher instance.
 */
void CommandDispatcher_Poll(CommandDispatcher_t *self);

/**
 * @brief Returns true if a decoded command is waiting to be consumed.
 *
 * @param self  Dispatcher instance.
 * @return true   A command is pending; call CommandDispatcher_GetCommand().
 * @return false  No command available.
 */
bool CommandDispatcher_HasCommand(const CommandDispatcher_t *self);

/**
 * @brief Returns a pointer to the pending command and clears the pending flag.
 *        The returned pointer remains valid until the next successful decode.
 *
 * @param self  Dispatcher instance.
 * @return Pointer to the last decoded PuzzleCommand.
 */
PuzzleCommand *CommandDispatcher_GetCommand(CommandDispatcher_t *self);

/**
 * @brief Encodes and sends an Ack message to the host over UART.
 *
 * @param self      Dispatcher instance.
 * @param status    Status code to include in the Ack (Status_STATUS_OK, etc.).
 * @param piece_id  Piece identifier to include in the Ack (0 for generic acks).
 */
void CommandDispatcher_SendAck(CommandDispatcher_t *self, Status status,
                               uint32_t piece_id);

#endif /* __COMMAND_DISPATCHER_H__ */
