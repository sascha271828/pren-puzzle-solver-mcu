#include "main.h"

#include "test_cli.h"

#include "homer.h"
#include "interrupt.h"
#include "magnet.h"
#include "piston.h"
#include "rotator.h"
#include "state_machine.h"
#include "step_generator.h"
#include "sys_config.h"
#include "sys_init.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Configuration ─────────────────────────────────────────────────────── */

#define CLI_LINE_BUF_SIZE 64
#define CLI_HOMING_TIMEOUT_MS 30000

/* ── Internal state ─────────────────────────────────────────────────────── */

static UART_HandleTypeDef* cli_huart = NULL;

/* ── Low-level I/O ──────────────────────────────────────────────────────── */

static void cli_putstr(const char* s) {
  HAL_UART_Transmit(cli_huart, (const uint8_t*)s, strlen(s), HAL_MAX_DELAY);
}

static void cli_ok(void) { cli_putstr("OK\r\n"); }

static void cli_busy(void) { cli_putstr("BUSY\r\n"); }

static void cli_err(const char* reason) {
  cli_putstr("ERR: ");
  cli_putstr(reason);
  cli_putstr("\r\n");
}

/**
 * @brief Reads one line from UART into buf (blocking).
 * Accepts \n or \r\n as line terminator.
 * Echoes each character back to the terminal.
 * Returns number of characters in the line (excluding terminator).
 */
static uint16_t cli_readline(char* buf, uint16_t max_len) {
  uint16_t idx = 0;
  uint8_t byte;

  while (1) {
    HAL_UART_Receive(cli_huart, &byte, 1, HAL_MAX_DELAY);

    if (byte == '\r') {
      continue; /* skip CR, wait for LF */
    }
    if (byte == '\n') {
      buf[idx] = '\0';
      cli_putstr("\r\n"); /* echo newline */
      return idx;
    }
    if (byte == 0x7F || byte == '\b') { /* backspace */
      if (idx > 0) {
        idx--;
        cli_putstr("\b \b");
      }
      continue;
    }

    /* echo printable characters */
    HAL_UART_Transmit(cli_huart, &byte, 1, HAL_MAX_DELAY);

    if (idx < max_len - 1) {
      buf[idx++] = (char)byte;
    }
  }
}

/* ── Busy-wait helpers ──────────────────────────────────────────────────── */

static void cli_wait_step_generator(void) {
  while (StepGenerator_IsBusy()) {
  }
}

static void cli_wait_rotator(void) {
  while (Rotator_IsBusy()) {
  }
}

static void cli_wait_piston(void) {
  while (Piston_IsBusy()) {
  }
}

/**
 * @brief Waits for homing to complete with a timeout.
 * @return true  Homing finished within timeout.
 * @return false Timeout exceeded — likely a hardware or config issue.
 */
static bool cli_wait_homing(void) {
  uint32_t start = HAL_GetTick();
  while (Interrupt_GetState() == IS_HOMING) {
    if (HAL_GetTick() - start > CLI_HOMING_TIMEOUT_MS) {
      return false;
    }
  }
  return true;
}

/* ── Command handlers ───────────────────────────────────────────────────── */

static void cmd_help(void) {
  cli_putstr("Commands:\r\n");
  cli_putstr("  ?              This help\r\n");
  cli_putstr("  s              Status\r\n");
  cli_putstr("  h              Home X and Y axes\r\n");
  cli_putstr("  m <x> <y>      Move X/Y (steps, signed)\r\n");
  cli_putstr("  r <steps>      Move rotator (steps, signed)\r\n");
  cli_putstr("  p <0..3>       Piston retract/extend\r\n");
  cli_putstr("  g <0|1>        Magnet off/on\r\n");
  cli_putstr(
      "  t              Testmachine sequence (with Hardware Button)\r\n");
}

static void cmd_status(void) {
  char buf[80];
  const char* sys_state;

  switch (Interrupt_GetState()) {
    case IS_INIT:
      sys_state = "INIT";
      break;
    case IS_HOMING:
      sys_state = "HOMING";
      break;
    case IS_READY:
      sys_state = "READY";
      break;
    case IS_RUNNING:
      sys_state = "RUNNING";
      break;
    case IS_ESTOP:
      sys_state = "ESTOP";
      break;
    default:
      sys_state = "UNKNOWN";
      break;
  }

  snprintf(buf, sizeof(buf), "SYS:  %s\r\n", sys_state);
  cli_putstr(buf);

  snprintf(buf,
           sizeof(buf),
           "XY:   %s\r\n",
           StepGenerator_IsBusy() ? "busy" : "idle");
  cli_putstr(buf);

  snprintf(
      buf, sizeof(buf), "ROT:  %s\r\n", Rotator_IsBusy() ? "busy" : "idle");
  cli_putstr(buf);

  snprintf(buf, sizeof(buf), "PST:  %s\r\n", Piston_IsBusy() ? "busy" : "idle");
  cli_putstr(buf);
}

static void cmd_home(void) {
  if (Interrupt_GetState() == IS_ESTOP) {
    cli_err("emergency stop active");
    return;
  }
  if (StepGenerator_IsBusy()) {
    cli_busy();
    return;
  }

  Homer_HomingStart();

  if (!cli_wait_homing()) {
    cli_err("homing timeout");
    return;
  }
  cli_ok();
}

static void cmd_move(const char* args) {
  int32_t x, y;
  if (sscanf(args, "%ld %ld", &x, &y) != 2) {
    cli_err("usage: m <x> <y>");
    return;
  }
  if (Interrupt_GetState() != IS_READY && Interrupt_GetState() != IS_RUNNING) {
    cli_err("not ready — home first");
    return;
  }
  if (StepGenerator_IsBusy()) {
    cli_busy();
    return;
  }

  static MoveBlock_t block; /* static: outlives StartMove */
  block = StepGenerator_GenerateBlock(x, y);

  if (!StepGenerator_StartMove(&block)) {
    cli_busy();
    return;
  }

  cli_wait_step_generator();
  cli_ok();
}

static void cmd_rotate(const char* args) {
  int32_t steps;
  if (sscanf(args, "%ld", &steps) != 1) {
    cli_err("usage: r <steps>");
    return;
  }
  if (Rotator_IsBusy()) {
    cli_busy();
    return;
  }

  static RotateBlock_t block; /* static: outlives StartMove */
  block = Rotator_GenerateBlock(steps);

  if (!Rotator_StartMove(&block)) {
    cli_busy();
    return;
  }

  cli_wait_rotator();
  cli_ok();
}

static void cmd_piston(const char* args) {
  int val;
  if (sscanf(args, "%d", &val) != 1 || (val < 0 && val >= PISTON_POS_COUNT)) {
    cli_err("usage: p <0..3>");
    return;
  }
  if (Piston_IsBusy()) {
    cli_busy();
    return;
  }

  Piston_Set((PistonLogical_e)val);
  cli_wait_piston();
  cli_ok();
}

static void cmd_magnet(const char* args) {
  int val;
  if (sscanf(args, "%d", &val) != 1 || (val != 0 && val != 1)) {
    cli_err("usage: g <0|1>");
    return;
  }

  Magnet_SetState((bool)val);
  cli_ok();
}

static void cmd_testmachine_sequence(void) {
  cli_putstr("Starting State Machine...\r\n");

  CommandDispatcher_t* dispatcher = Sys_GetCommandDispatcher();
  Magnet_t* magnet = Sys_GetMagnet();
  StateMachine_Init(dispatcher, magnet);

  cli_putstr(
      "Waiting for homing to finish, then waiting for hardware START "
      "button...\r\n");

  bool data_injected = false;

  while (1) {
    StateMachine_Update();

    if (Interrupt_GetState() == IS_ESTOP) {
      cli_err("Emergency Stop triggered!");
      return;
    }

    if (StateMachine_IsIdle() && !data_injected) {
      cli_putstr("Start button pressed! Injecting test data...\r\n");

      PuzzleCommand test_cmd = PuzzleCommand_init_zero;
      test_cmd.pieces_count = 2;

      /* Piece 1 */
      test_cmd.pieces[0].piece_id = 1;
      test_cmd.pieces[0].pick_x = 30.0f;
      test_cmd.pieces[0].pick_y = 30.0f;
      test_cmd.pieces[0].place_x = 40.0f;
      test_cmd.pieces[0].place_y = 40.0f;
      test_cmd.pieces[0].rotation = 90.0f;

      /* Piece 2 */
      test_cmd.pieces[1].piece_id = 2;
      test_cmd.pieces[1].pick_x = 100.0f;
      test_cmd.pieces[1].pick_y = 100.0f;
      test_cmd.pieces[1].place_x = 180.0f;
      test_cmd.pieces[1].place_y = 200.0f;
      test_cmd.pieces[1].rotation = -45.0f;

      StateMachine_StartManual(&test_cmd);
      data_injected = true;
    }

    if (StateMachine_IsIdle() && data_injected) {
      cli_putstr("Test sequence finished successfully.\r\n");
      cli_ok();
      return;
    }
  }
}

/* ── Dispatcher ─────────────────────────────────────────────────────────── */

/**
 * @brief Parses and dispatches one null-terminated command line.
 * cmd points at the first non-space character of the line.
 * args points to the remainder after the command character and
 * any whitespace (may be an empty string "").
 */
static void cli_dispatch(char* line) {
  /* skip leading whitespace */
  while (*line == ' ') line++;

  if (*line == '\0') return; /* empty line */

  char cmd = *line;

  /* args: skip cmd char + any spaces */
  char* args = line + 1;
  while (*args == ' ') args++;

  switch (cmd) {
    case '?':
      cmd_help();
      break;
    case 's':
      cmd_status();
      break;
    case 'h':
      cmd_home();
      break;
    case 'm':
      cmd_move(args);
      break;
    case 'r':
      cmd_rotate(args);
      break;
    case 'p':
      cmd_piston(args);
      break;
    case 'g':
      cmd_magnet(args);
      break;
    case 't':
      cmd_testmachine_sequence();
      break;
    default:
      cli_err("unknown command — type ? for help");
      break;
  }
}

/* ── Public API ─────────────────────────────────────────────────────────── */

void TestCLI_Init(UART_HandleTypeDef* huart) { cli_huart = huart; }

void TestCLI_Run(void) {
  char line[CLI_LINE_BUF_SIZE];

  cli_putstr("\r\nBOOT\r\n");
  cli_putstr("=== PuzzleSolver Test CLI ===\r\n");
  cli_putstr("Type ? for help\r\n");
  cli_putstr("\r\n");

  for (;;) {
    cli_putstr("> ");
    cli_readline(line, sizeof(line));
    cli_dispatch(line);
  }
}