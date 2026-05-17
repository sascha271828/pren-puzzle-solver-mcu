#ifndef __TEST_CLI_H__
#define __TEST_CLI_H__

/**
 * @file test_cli.h
 * @brief Interactive ASCII command-line interface for subsystem testing.
 *
 * Receives single-line ASCII commands over any UART instance and dispatches
 * them to the subsystem APIs.  Intended for development and integration
 * testing only; excluded from production builds via RUN_MODE in sys_config.h.
 *
 * Command format:  <cmd> [arg1] [arg2]\n
 * Response format: OK\r\n | ERR: <reason>\r\n | BUSY\r\n
 *
 * Available commands:
 *
 *   Motion (home required first):
 *     h                   Home X and Y axes (blocks, 30 s timeout)
 *     m <x> <y>           Move X/Y by relative step count (signed int32)
 *     r <steps>           Move rotator by step count (signed int32)
 *     b <x> <y>           Move to absolute position in µm (via motion planner)
 *
 *   Actuators:
 *     p <0..3>            Piston: 0=START  1=MOVE  2=GRAB  3=RELEASE
 *     g <0|1>             Magnet: 0=off  1=on
 *
 *   State Machine Test:
 *     x                   Run default 2-piece test sequence (blocks, 5 min timeout)
 *     x <px> <py> <plx> <ply> [rot]
 *                         Run 1-piece sequence with custom coordinates [mm, degrees]
 *
 *   LEDs:
 *     l <0|1>             Work-area LED: 0=off  1=on
 *     a <n>               Status LED — n encodes LED and mode:
 *                           0-9  = green   (0=off, 1=blink, 2=on)
 *                           10-19= yellow  (10=off, 11=blink, 12=on)
 *                           20-29= red     (20=off, 21=blink, 22=on)
 *
 *   Info:
 *     s                   Full system status (state, actuators, limits, LEDs)
 *     ?                   Print this help
 */

#include "main.h"

/**
 * @brief Initialises the CLI with the UART instance to use and prepares
 *        the internal state machine dispatcher for the 'x' command.
 *        Must be called once before TestCLI_Run().
 *
 * @param huart  Pointer to an initialised UART_HandleTypeDef.
 *               On Nucleo-H753ZI use &huart3 (ST-Link Virtual COM Port).
 *
 * @note  When the state machine completes a test sequence ('x' command),
 *        it sends a binary Ack frame over this UART. A few garbled bytes
 *        may appear in the terminal at that point — this is expected.
 */
void TestCLI_Init(UART_HandleTypeDef* huart);

/**
 * @brief Blocking CLI loop — never returns.
 *        Prints a welcome banner, then waits for commands line by line.
 *        Each command executes synchronously; a response is sent before
 *        the next command is accepted.
 *
 * @note  Call from App_Run() when RUN_MODE == RUN_MODE_TEST_CLI.
 * @note  The 'x' command blocks for up to 5 minutes while driving the
 *        full state machine sequence; all other commands block only for
 *        the duration of the physical move.
 */
void TestCLI_Run(void);

#endif /* __TEST_CLI_H__ */