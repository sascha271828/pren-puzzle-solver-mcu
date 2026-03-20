/*
 * Helper for cross-language test.
 * Reads a length-prefixed protobuf frame from stdin,
 * decodes it, prints the values to stdout,
 * and writes the Ack frame to stderr (as raw bytes).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"

/* Capture TX for ack — write to stderr as raw bytes */
int HAL_UART_Receive_IT(UART_HandleTypeDef *huart, uint8_t *pData,
                        uint16_t Size) {
  (void)huart;
  (void)pData;
  (void)Size;
  return HAL_OK;
}

int HAL_UART_Transmit(UART_HandleTypeDef *huart, const uint8_t *pData,
                      uint16_t Size, uint32_t Timeout) {
  (void)huart;
  (void)Timeout;
  fwrite(pData, 1, Size, stderr);
  fflush(stderr);
  return HAL_OK;
}

#include "puzzle.pb.h"
#include "uart_receiver.h"
#include "command_dispatcher.h"
#include <pb_decode.h>

int main(void) {
  /* Read frame from stdin */
  uint8_t frame[512];
  size_t n = fread(frame, 1, sizeof(frame), stdin);
  if (n < 2) {
    fprintf(stdout, "ERROR: too few bytes (%zu)\n", n);
    return 1;
  }

  /* Set up receiver and dispatcher */
  UART_HandleTypeDef fake_uart = {.Instance = 7};
  UartReceiver_t rx;
  CommandDispatcher_t disp;
  UartReceiver_Init(&rx, &fake_uart);
  CommandDispatcher_Init(&disp, &rx);
  UartReceiver_Start(&rx);

  /* Feed bytes */
  for (size_t i = 0; i < n; i++) {
    rx.rx_byte = frame[i];
    UartReceiver_RxCallback(&rx);
  }

  /* Decode */
  CommandDispatcher_Poll(&disp);

  if (!CommandDispatcher_HasCommand(&disp)) {
    fprintf(stdout, "ERROR: no command decoded\n");
    return 1;
  }

  PuzzleCommand *cmd = CommandDispatcher_GetCommand(&disp);

  fprintf(stdout, "pieces_count=%d\n", (int)cmd->pieces_count);
  for (int i = 0; i < (int)cmd->pieces_count; i++) {
    PieceCommand *p = &cmd->pieces[i];
    fprintf(stdout,
            "  [%d] piece_id=%u pick_x=%.1f pick_y=%.1f "
            "place_x=%.1f place_y=%.1f rotation=%.1f\n",
            i, (unsigned)p->piece_id, p->pick_x, p->pick_y, p->place_x,
            p->place_y, p->rotation);
  }
  fflush(stdout);

  return 0;
}
