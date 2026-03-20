#include "uart_receiver.h"

#include <string.h>

void UartReceiver_Init(UartReceiver_t *self, UART_HandleTypeDef *huart) {
  memset(self, 0, sizeof(*self));
  self->huart = huart;
  self->reading_header = true;
}

void UartReceiver_Start(UartReceiver_t *self) {
  self->reading_header = true;
  self->header_idx = 0;
  self->buf_idx = 0;
  self->frame_ready = false;
  HAL_UART_Receive_IT(self->huart, &self->rx_byte, 1);
}

void UartReceiver_RxCallback(UartReceiver_t *self) {
  if (self->frame_ready) {
    /* Previous frame not consumed yet, re-arm and drop */
    HAL_UART_Receive_IT(self->huart, &self->rx_byte, 1);
    return;
  }

  if (self->reading_header) {
    self->header_bytes[self->header_idx++] = self->rx_byte;
    if (self->header_idx >= 2) {
      self->expected_len =
          ((uint16_t)self->header_bytes[0] << 8) | self->header_bytes[1];
      if (self->expected_len > UART_RX_BUF_SIZE) {
        /* Invalid length, reset */
        self->header_idx = 0;
      } else {
        self->reading_header = false;
        self->buf_idx = 0;
      }
    }
  } else {
    self->buf[self->buf_idx++] = self->rx_byte;
    if (self->buf_idx >= self->expected_len) {
      self->frame_ready = true;
      /* Don't re-arm here; GetPayload will re-arm after consuming */
      return;
    }
  }

  HAL_UART_Receive_IT(self->huart, &self->rx_byte, 1);
}

bool UartReceiver_IsFrameReady(const UartReceiver_t *self) {
  return self->frame_ready;
}

size_t UartReceiver_GetPayload(UartReceiver_t *self, uint8_t *out,
                               size_t max_len) {
  if (!self->frame_ready) return 0;

  size_t copy_len =
      self->expected_len < max_len ? self->expected_len : max_len;
  memcpy(out, self->buf, copy_len);

  /* Reset and re-arm for next frame */
  self->frame_ready = false;
  self->reading_header = true;
  self->header_idx = 0;
  self->buf_idx = 0;
  HAL_UART_Receive_IT(self->huart, &self->rx_byte, 1);

  return copy_len;
}

void UartReceiver_SendRaw(UartReceiver_t *self, const uint8_t *data,
                          size_t len) {
  HAL_UART_Transmit(self->huart, data, (uint16_t)len, HAL_MAX_DELAY);
}

void UartReceiver_SendFrame(UartReceiver_t *self, const uint8_t *payload,
                            size_t len) {
  uint8_t header[2];
  header[0] = (uint8_t)(len >> 8);
  header[1] = (uint8_t)(len & 0xFF);
  HAL_UART_Transmit(self->huart, header, 2, HAL_MAX_DELAY);
  HAL_UART_Transmit(self->huart, payload, (uint16_t)len, HAL_MAX_DELAY);
}
