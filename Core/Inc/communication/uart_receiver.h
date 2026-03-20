#ifndef __UART_RECEIVER_H__
#define __UART_RECEIVER_H__

#include "main.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Length-prefix framing: [len_hi] [len_lo] [payload...] */
#define UART_RX_BUF_SIZE (512)

typedef struct {
  UART_HandleTypeDef *huart;
  uint8_t rx_byte;
  uint8_t buf[UART_RX_BUF_SIZE];
  uint16_t expected_len;
  uint16_t buf_idx;
  bool reading_header;
  uint8_t header_bytes[2];
  uint8_t header_idx;
  volatile bool frame_ready;
} UartReceiver_t;

void UartReceiver_Init(UartReceiver_t *self, UART_HandleTypeDef *huart);
void UartReceiver_Start(UartReceiver_t *self);
void UartReceiver_RxCallback(UartReceiver_t *self);
bool UartReceiver_IsFrameReady(const UartReceiver_t *self);
size_t UartReceiver_GetPayload(UartReceiver_t *self, uint8_t *out,
                               size_t max_len);

/* Call from HAL_UART_TxCpltCallback if needed */
void UartReceiver_SendRaw(UartReceiver_t *self, const uint8_t *data,
                          size_t len);
/* Send a length-prefixed frame */
void UartReceiver_SendFrame(UartReceiver_t *self, const uint8_t *payload,
                            size_t len);

#endif /* __UART_RECEIVER_H__ */
