/*
 * Minimal main.h stub for native (non-STM32) test compilation.
 * Provides just the types that our communication code needs.
 */
#ifndef __MAIN_H_STUB__
#define __MAIN_H_STUB__

#include <stdint.h>
#include <stddef.h>

typedef struct {
  int Instance;
} UART_HandleTypeDef;

#define HAL_MAX_DELAY 0xFFFFFFFF
#define HAL_OK 0

int HAL_UART_Receive_IT(UART_HandleTypeDef *huart, uint8_t *pData,
                        uint16_t Size);
int HAL_UART_Transmit(UART_HandleTypeDef *huart, const uint8_t *pData,
                      uint16_t Size, uint32_t Timeout);

#endif /* __MAIN_H_STUB__ */
