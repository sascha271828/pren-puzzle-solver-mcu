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
 *   s              Print status of all axes and system state
 *   h              Start homing sequence (blocks until done or timeout)
 *   m <x> <y>      Move X/Y axes by relative step count (signed int32)
 *   r <steps>      Move rotator by step count (signed int32)
 *   p <0..3>       Set piston position (0=START, 1=MOVE, 2=GRAB, 3=RELEASE)
 *   g <0|1>        Magnet off (0) / on (1)
 *   l <0|1>        Work-area LED off (0) / on (1)
 *   a <0..29>      Status LED control: 0-9=green, 10-19=yellow, 20-29=red;
 *                  last digit selects mode: 0=off, 1=blink, 2=on
 *   b <x> <y>      Move to absolute position in µm (converts to steps)
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