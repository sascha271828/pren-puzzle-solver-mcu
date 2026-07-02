#ifndef __UART_RECEIVER_H__
#define __UART_RECEIVER_H__

/**
 * @file uart_receiver.h
 * @brief Interrupt-driven UART receiver with length-prefix framing.
 *
 * Receives binary frames over UART using HAL interrupt mode (byte-by-byte).
 * Frame format: [len_hi] [len_lo] [payload bytes...] — big-endian 16-bit
 * length header followed by exactly that many payload bytes.
 *
 * Receive flow:
 *  1. Call UartReceiver_Init() + UartReceiver_Start() during system init.
 *  2. Route HAL_UART_RxCpltCallback() → UartReceiver_RxCallback().
 *  3. In the main loop, poll UartReceiver_IsFrameReady().
 *  4. When ready, call UartReceiver_GetPayload() to copy the payload and
 *     re-arm reception.
 *
 * If a frame arrives before the previous one is consumed, the new bytes are
 * silently dropped and reception is re-armed.
 *
 * Transmit functions (SendRaw / SendFrame) use blocking HAL_UART_Transmit
 * with HAL_MAX_DELAY and are safe to call from the main context.
 */

#include "main.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/** @brief Maximum payload size in bytes (limits expected_len check). */
#define UART_RX_BUF_SIZE (512)

/** @brief Silence on the line after which a partial frame is discarded. */
#define UART_RX_FRAME_TIMEOUT_MS (100)

/**
 * @brief Runtime state for one UART receiver instance.
 *        All fields are managed by the module; treat as private.
 */
typedef struct {
  UART_HandleTypeDef *huart;       /**< HAL UART handle                        */
  uint8_t rx_byte;                 /**< Single-byte DMA/IT transfer target     */
  uint8_t buf[UART_RX_BUF_SIZE];  /**< Payload accumulation buffer            */
  uint16_t expected_len;           /**< Payload length decoded from header     */
  uint16_t buf_idx;                /**< Current write index into buf[]         */
  bool reading_header;             /**< True while collecting the 2-byte header*/
  uint8_t header_bytes[2];         /**< Raw header bytes [len_hi, len_lo]      */
  uint8_t header_idx;              /**< Number of header bytes received so far */
  volatile bool frame_ready;       /**< True when a complete frame is in buf[] */
  volatile uint32_t last_rx_tick;  /**< HAL tick of the most recent RX byte    */
} UartReceiver_t;

/**
 * @brief Initialises the receiver state and stores the UART handle.
 *        Must be called before UartReceiver_Start().
 *
 * @param self   Receiver instance to initialise.
 * @param huart  Pointer to an HAL-initialised UART handle.
 */
void UartReceiver_Init(UartReceiver_t *self, UART_HandleTypeDef *huart);

/**
 * @brief Arms the first byte-by-byte interrupt transfer.
 *        Must be called once after UartReceiver_Init() to begin reception.
 *
 * @param self  Receiver instance.
 */
void UartReceiver_Start(UartReceiver_t *self);

/**
 * @brief Processes one received byte. Must be called from
 *        HAL_UART_RxCpltCallback() when the matching UART fires.
 *
 * @param self  Receiver instance.
 */
void UartReceiver_RxCallback(UartReceiver_t *self);

/**
 * @brief Discards a partially received frame if the line has been silent
 *        for more than UART_RX_FRAME_TIMEOUT_MS. Call periodically from the
 *        main loop; without this, a single stray byte desyncs the framing
 *        permanently (parser waits forever for a bogus payload length).
 *
 * @param self  Receiver instance.
 */
void UartReceiver_CheckTimeout(UartReceiver_t *self);

/**
 * @brief Returns true if a complete frame has been received and is
 *        waiting to be consumed by UartReceiver_GetPayload().
 *
 * @param self  Receiver instance.
 * @return true   Frame ready.
 * @return false  No complete frame yet.
 */
bool UartReceiver_IsFrameReady(const UartReceiver_t *self);

/**
 * @brief Copies the payload of the pending frame into @p out, clears
 *        the frame_ready flag, and re-arms reception.
 *
 * @param self     Receiver instance.
 * @param out      Destination buffer; must be at least @p max_len bytes.
 * @param max_len  Maximum number of bytes to copy.
 * @return Number of bytes copied (≤ max_len). Returns 0 if no frame is ready.
 */
size_t UartReceiver_GetPayload(UartReceiver_t *self, uint8_t *out,
                               size_t max_len);

/**
 * @brief Transmits raw bytes over UART (blocking, HAL_MAX_DELAY).
 *
 * @param self  Receiver instance.
 * @param data  Pointer to the data buffer.
 * @param len   Number of bytes to transmit.
 */
void UartReceiver_SendRaw(UartReceiver_t *self, const uint8_t *data,
                          size_t len);

/**
 * @brief Encodes and transmits a length-prefixed frame (blocking).
 *        Sends [len_hi][len_lo][payload...] using two HAL_UART_Transmit calls.
 *
 * @param self     Receiver instance.
 * @param payload  Pointer to the payload data.
 * @param len      Payload length in bytes.
 */
void UartReceiver_SendFrame(UartReceiver_t *self, const uint8_t *payload,
                            size_t len);

#endif /* __UART_RECEIVER_H__ */
