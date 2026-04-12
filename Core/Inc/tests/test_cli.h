#ifndef __TEST_CLI_H__
#define __TEST_CLI_H__

/**
 * @file test_cli.h
 * @brief Interactive ASCII command-line interface for subsystem testing.
 *
 * Receives single-line ASCII commands over any UART instance and dispatches
 * them directly to the subsystem APIs.  Intended for use during development
 * only; excluded from production builds via RUN_MODE in sys_config.h.
 *
 * Command format:  <cmd> [arg1] [arg2]\n
 * Response format: OK\r\n | ERR: <reason>\r\n | BUSY\r\n
 *
 * Available commands:
 *   ?              Print help
 *   s              Print status of all axes
 *   h              Start homing sequence
 *   m <x> <y>      Move X/Y axes (steps, signed int32)
 *   r <steps>      Move rotator  (steps, signed int32)
 *   p <0|1>        Piston retract (0) / extend (1)
 *   g <0|1>        Magnet off (0) / on (1)
 */

#include "main.h"

/**
 * @brief Initialises the CLI with the UART instance to use.
 *        Must be called once before TestCLI_Run().
 *
 * @param huart  Pointer to an initialised UART_HandleTypeDef.
 *               On Nucleo-H753ZI use &huart3 (ST-Link Virtual COM Port).
 */
void TestCLI_Init(UART_HandleTypeDef* huart);

/**
 * @brief Blocking CLI loop — never returns.
 *        Prints a welcome banner, then waits for commands line by line.
 *        Each command executes synchronously; a response is sent before
 *        the next command is accepted.
 *
 * @note  Call from App_Run() when RUN_MODE == RUN_MODE_TEST_CLI.
 */
void TestCLI_Run(void);

#endif /* __TEST_CLI_H__ */