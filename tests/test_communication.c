/*
 * Native test for the communication pipeline (no STM32 hardware needed).
 *
 * Mocks HAL_UART functions and simulates the full roundtrip:
 *   Python-side encode → frame → byte-by-byte feed → decode → verify
 *
 * Compile & run:
 *   cc -o test_comm tests/test_communication.c \
 *      Core/Src/communication/uart_receiver.c \
 *      Core/Src/communication/command_dispatcher.c \
 *      Core/Src/communication/puzzle.pb.c \
 *      Core/third_party/nanopb/pb_common.c \
 *      Core/third_party/nanopb/pb_decode.c \
 *      Core/third_party/nanopb/pb_encode.c \
 *      -ICore/Inc/communication \
 *      -ICore/third_party/nanopb \
 *      -Itests \
 *      -lm
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"  /* tests/main.h stub */

/* ---- HAL stub implementations ---- */

/* Capture TX output for verification */
static uint8_t tx_capture[512];
static size_t tx_capture_len = 0;

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
  memcpy(tx_capture + tx_capture_len, pData, Size);
  tx_capture_len += Size;
  return HAL_OK;
}

#define UART7 7

#include "puzzle.pb.h"
#include "uart_receiver.h"
#include "command_dispatcher.h"
#include <pb_encode.h>
#include <pb_decode.h>

/* ---- Helpers ---- */

static void feed_bytes(UartReceiver_t *rx, const uint8_t *data, size_t len) {
  for (size_t i = 0; i < len; i++) {
    rx->rx_byte = data[i];
    UartReceiver_RxCallback(rx);
  }
}

static int float_eq(float a, float b) {
  return fabsf(a - b) < 0.001f;
}

/* ---- Tests ---- */

static int test_roundtrip(void) {
  printf("test_roundtrip: ");

  /* 1. Encode a PuzzleCommand (simulating what Python sends) */
  PuzzleCommand cmd = PuzzleCommand_init_zero;
  cmd.pieces_count = 3;

  cmd.pieces[0].piece_id = 0;
  cmd.pieces[0].pick_x = 100.5f;
  cmd.pieces[0].pick_y = 200.0f;
  cmd.pieces[0].place_x = 50.0f;
  cmd.pieces[0].place_y = 300.0f;
  cmd.pieces[0].rotation = 90.0f;

  cmd.pieces[1].piece_id = 1;
  cmd.pieces[1].pick_x = 150.0f;
  cmd.pieces[1].pick_y = 250.0f;
  cmd.pieces[1].place_x = 75.0f;
  cmd.pieces[1].place_y = 350.0f;
  cmd.pieces[1].rotation = 180.0f;

  cmd.pieces[2].piece_id = 2;
  cmd.pieces[2].pick_x = 0.0f;
  cmd.pieces[2].pick_y = 0.0f;
  cmd.pieces[2].place_x = 420.0f;
  cmd.pieces[2].place_y = 594.0f;
  cmd.pieces[2].rotation = 270.0f;

  uint8_t pb_buf[PuzzleCommand_size];
  pb_ostream_t ostream = pb_ostream_from_buffer(pb_buf, sizeof(pb_buf));
  if (!pb_encode(&ostream, PuzzleCommand_fields, &cmd)) {
    printf("FAIL (encode: %s)\n", PB_GET_ERROR(&ostream));
    return 1;
  }
  size_t pb_len = ostream.bytes_written;

  /* 2. Build a length-prefixed frame (what Python's _send_frame does) */
  uint8_t frame[2 + PuzzleCommand_size];
  frame[0] = (uint8_t)(pb_len >> 8);
  frame[1] = (uint8_t)(pb_len & 0xFF);
  memcpy(frame + 2, pb_buf, pb_len);
  size_t frame_len = 2 + pb_len;

  /* 3. Set up the C-side receiver and dispatcher */
  UART_HandleTypeDef fake_uart = {.Instance = UART7};
  UartReceiver_t rx;
  CommandDispatcher_t disp;

  UartReceiver_Init(&rx, &fake_uart);
  CommandDispatcher_Init(&disp, &rx);
  UartReceiver_Start(&rx);

  /* 4. Feed the frame byte by byte (simulating UART interrupts) */
  feed_bytes(&rx, frame, frame_len);

  if (!UartReceiver_IsFrameReady(&rx)) {
    printf("FAIL (frame not ready after feeding %zu bytes)\n", frame_len);
    return 1;
  }

  /* 5. Let the dispatcher decode it */
  tx_capture_len = 0;  /* reset TX capture for ack */
  CommandDispatcher_Poll(&disp);

  if (!CommandDispatcher_HasCommand(&disp)) {
    printf("FAIL (no command after poll)\n");
    return 1;
  }

  /* 6. Verify decoded data */
  PuzzleCommand *result = CommandDispatcher_GetCommand(&disp);

  if (result->pieces_count != 3) {
    printf("FAIL (pieces_count=%d, expected 3)\n", (int)result->pieces_count);
    return 1;
  }

  if (result->pieces[0].piece_id != 0 ||
      !float_eq(result->pieces[0].pick_x, 100.5f) ||
      !float_eq(result->pieces[0].pick_y, 200.0f) ||
      !float_eq(result->pieces[0].place_x, 50.0f) ||
      !float_eq(result->pieces[0].place_y, 300.0f) ||
      !float_eq(result->pieces[0].rotation, 90.0f)) {
    printf("FAIL (piece 0 data mismatch)\n");
    return 1;
  }

  if (result->pieces[2].piece_id != 2 ||
      !float_eq(result->pieces[2].place_x, 420.0f) ||
      !float_eq(result->pieces[2].place_y, 594.0f) ||
      !float_eq(result->pieces[2].rotation, 270.0f)) {
    printf("FAIL (piece 2 data mismatch)\n");
    return 1;
  }

  /* 7. Verify the Ack was sent back */
  if (tx_capture_len == 0) {
    printf("FAIL (no ack sent)\n");
    return 1;
  }

  /* Ack frame: [len_hi][len_lo][protobuf] */
  uint16_t ack_len = ((uint16_t)tx_capture[0] << 8) | tx_capture[1];
  Ack ack = Ack_init_zero;
  pb_istream_t istream =
      pb_istream_from_buffer(tx_capture + 2, ack_len);
  if (!pb_decode(&istream, Ack_fields, &ack)) {
    printf("FAIL (ack decode: %s)\n", PB_GET_ERROR(&istream));
    return 1;
  }

  if (ack.status != Status_STATUS_OK) {
    printf("FAIL (ack status=%d, expected OK)\n", (int)ack.status);
    return 1;
  }

  printf("PASS\n");
  return 0;
}

static int test_partial_feed(void) {
  printf("test_partial_feed: ");

  /* Encode a small command */
  PuzzleCommand cmd = PuzzleCommand_init_zero;
  cmd.pieces_count = 1;
  cmd.pieces[0].piece_id = 42;
  cmd.pieces[0].pick_x = 1.0f;

  uint8_t pb_buf[PuzzleCommand_size];
  pb_ostream_t ostream = pb_ostream_from_buffer(pb_buf, sizeof(pb_buf));
  pb_encode(&ostream, PuzzleCommand_fields, &cmd);
  size_t pb_len = ostream.bytes_written;

  uint8_t frame[2 + PuzzleCommand_size];
  frame[0] = (uint8_t)(pb_len >> 8);
  frame[1] = (uint8_t)(pb_len & 0xFF);
  memcpy(frame + 2, pb_buf, pb_len);
  size_t frame_len = 2 + pb_len;

  UART_HandleTypeDef fake_uart = {.Instance = UART7};
  UartReceiver_t rx;
  UartReceiver_Init(&rx, &fake_uart);
  UartReceiver_Start(&rx);

  /* Feed first half */
  size_t half = frame_len / 2;
  feed_bytes(&rx, frame, half);

  if (UartReceiver_IsFrameReady(&rx)) {
    printf("FAIL (frame ready too early)\n");
    return 1;
  }

  /* Feed second half */
  feed_bytes(&rx, frame + half, frame_len - half);

  if (!UartReceiver_IsFrameReady(&rx)) {
    printf("FAIL (frame not ready after full feed)\n");
    return 1;
  }

  printf("PASS\n");
  return 0;
}

static int test_back_to_back_frames(void) {
  printf("test_back_to_back_frames: ");

  UART_HandleTypeDef fake_uart = {.Instance = UART7};
  UartReceiver_t rx;
  CommandDispatcher_t disp;
  UartReceiver_Init(&rx, &fake_uart);
  CommandDispatcher_Init(&disp, &rx);
  UartReceiver_Start(&rx);

  for (int i = 0; i < 3; i++) {
    PuzzleCommand cmd = PuzzleCommand_init_zero;
    cmd.pieces_count = 1;
    cmd.pieces[0].piece_id = (uint32_t)i;
    cmd.pieces[0].pick_x = (float)(i * 10);

    uint8_t pb_buf[PuzzleCommand_size];
    pb_ostream_t ostream = pb_ostream_from_buffer(pb_buf, sizeof(pb_buf));
    pb_encode(&ostream, PuzzleCommand_fields, &cmd);
    size_t pb_len = ostream.bytes_written;

    uint8_t frame[2 + PuzzleCommand_size];
    frame[0] = (uint8_t)(pb_len >> 8);
    frame[1] = (uint8_t)(pb_len & 0xFF);
    memcpy(frame + 2, pb_buf, pb_len);

    tx_capture_len = 0;
    feed_bytes(&rx, frame, 2 + pb_len);
    CommandDispatcher_Poll(&disp);

    if (!CommandDispatcher_HasCommand(&disp)) {
      printf("FAIL (no command on iteration %d)\n", i);
      return 1;
    }

    PuzzleCommand *result = CommandDispatcher_GetCommand(&disp);
    if (result->pieces[0].piece_id != (uint32_t)i) {
      printf("FAIL (piece_id=%u, expected %d)\n",
             (unsigned)result->pieces[0].piece_id, i);
      return 1;
    }
  }

  printf("PASS\n");
  return 0;
}

int main(void) {
  int failures = 0;
  printf("=== Communication Pipeline Tests ===\n\n");

  failures += test_roundtrip();
  failures += test_partial_feed();
  failures += test_back_to_back_frames();

  printf("\n%s (%d failures)\n",
         failures == 0 ? "ALL PASSED" : "SOME FAILED", failures);
  return failures;
}
