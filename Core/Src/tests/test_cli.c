#include "main.h"

#include "test_cli.h"

#include "buttons.h"
#include "command_dispatcher.h"
#include "emergency_stop.h"
#include "homer.h"
#include "interrupt.h"
#include "leds.h"
#include "limit_switch.h"
#include "magnet.h"
#include "motion_planner.h"
#include "piston.h"
#include "rotator.h"
#include "state_machine.h"
#include "status_leds.h"
#include "step_generator.h"
#include "sys_config.h"
#include "sys_init.h"
#include "uart_receiver.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ========================
 *   PRIVATE DEFINITIONS
 * ======================== */

#define CLI_LINE_BUF_SIZE    64
#define CLI_HOMING_TIMEOUT_MS  30000u
#define CLI_SM_TIMEOUT_MS    300000u  /* 5 min max for a full sequence */

/* ========================
 *   PRIVATE VARIABLES
 * ======================== */

static UART_HandleTypeDef* cli_huart = NULL;

/* Local absolute position tracker for the 'b' command */
static int32_t current_pos_steps_x = 0;
static int32_t current_pos_steps_y = 0;

/* Dispatcher used by the state machine in CLI test mode.
 * Ack messages (binary protobuf) will be sent over the CLI UART —
 * expect a few garbled bytes in the terminal when a sequence finishes. */
static UartReceiver_t     cli_sm_uart;
static CommandDispatcher_t cli_sm_dispatcher;

/* ========================
 *   PRIVATE FUNCTIONS — I/O helpers
 * ======================== */

static void cli_putstr(const char* s) {
  HAL_UART_Transmit(cli_huart, (const uint8_t*)s, strlen(s), HAL_MAX_DELAY);
}

static void cli_ok(void)   { cli_putstr("OK\r\n"); }
static void cli_busy(void) { cli_putstr("BUSY\r\n"); }

static void cli_err(const char* reason) {
  cli_putstr("ERR: ");
  cli_putstr(reason);
  cli_putstr("\r\n");
}

/* Reads one line from UART (blocking). Accepts \n or \r\n as terminator.
 * Echoes each character; handles backspace. Returns character count. */
static uint16_t cli_readline(char* buf, uint16_t max_len) {
  uint16_t idx = 0;
  uint8_t  byte;

  while (1) {
    HAL_UART_Receive(cli_huart, &byte, 1, HAL_MAX_DELAY);

    if (byte == '\r') continue;
    if (byte == '\n') {
      buf[idx] = '\0';
      cli_putstr("\r\n");
      return idx;
    }
    if (byte == 0x7F || byte == '\b') {
      if (idx > 0) { idx--; cli_putstr("\b \b"); }
      continue;
    }

    HAL_UART_Transmit(cli_huart, &byte, 1, HAL_MAX_DELAY);
    if (idx < max_len - 1) buf[idx++] = (char)byte;
  }
}

/* ========================
 *   PRIVATE FUNCTIONS — busy-wait helpers
 * ======================== */

static void cli_wait_step_generator(void) { while (StepGenerator_IsBusy()) {} }
static void cli_wait_rotator(void)        { while (Rotator_IsBusy()) {} }
static void cli_wait_piston(void)         { while (Piston_IsBusy()) {} }

/* Waits for homing to finish, returns false on timeout. */
static bool cli_wait_homing(void) {
  uint32_t start = HAL_GetTick();
  while (Interrupt_GetState() == IS_HOMING) {
    if (HAL_GetTick() - start > CLI_HOMING_TIMEOUT_MS) return false;
  }
  return true;
}

/* ========================
 *   PRIVATE FUNCTIONS — string helpers
 * ======================== */

static const char* interrupt_state_str(InterruptState_t s) {
  switch (s) {
    case IS_INIT:    return "INIT";
    case IS_HOMING:  return "HOMING";
    case IS_READY:   return "READY";
    case IS_RUNNING: return "RUNNING";
    case IS_ESTOP:   return "ESTOP";
    default:         return "UNKNOWN";
  }
}

static const char* led_type_str(StatusLeds_Type_e t) {
  switch (t) {
    case STATUSLED_TYPE_ON:    return "on";
    case STATUSLED_TYPE_BLINK: return "blink";
    default:                   return "off";
  }
}

static const char* busy_str(bool b) { return b ? "busy" : "idle"; }
static const char* on_off_str(bool b) { return b ? "on" : "off"; }

/* ========================
 *   PRIVATE FUNCTIONS — command handlers
 * ======================== */

static void cmd_help(void) {
  cli_putstr("\r\n=== PuzzleSolver Test CLI ===\r\n");
  cli_putstr("\r\n-- Motion (home required first) ----------------------------\r\n");
  cli_putstr("  h              Home X and Y axes\r\n");
  cli_putstr("  m <x> <y>      Move X/Y by step count (signed int32)\r\n");
  cli_putstr("  r <steps>      Move rotator by step count (signed int32)\r\n");
  cli_putstr("  b <x> <y>      Move to absolute position [um] (via planner)\r\n");
  cli_putstr("\r\n-- Actuators ------------------------------------------------\r\n");
  cli_putstr("  p <0..3>       Piston: 0=START  1=MOVE  2=GRAB  3=RELEASE\r\n");
  cli_putstr("  g <0|1>        Magnet: 0=off  1=on\r\n");
  cli_putstr("\r\n-- State Machine Test ---------------------------------------\r\n");
  cli_putstr("  x              Run default 2-piece test sequence\r\n");
  cli_putstr("  x <px> <py> <plx> <ply> [rot]\r\n");
  cli_putstr("                 Run 1-piece sequence with custom coords [mm, deg]\r\n");
  cli_putstr("\r\n-- LEDs -----------------------------------------------------\r\n");
  cli_putstr("  l <0|1>        Work-area LED: 0=off  1=on\r\n");
  cli_putstr("  a <n>          Status LED — n encodes LED + mode:\r\n");
  cli_putstr("                   0-9  = green   (0=off, 1=blink, 2=on)\r\n");
  cli_putstr("                   10-19= yellow  (10=off, 11=blink, 12=on)\r\n");
  cli_putstr("                   20-29= red     (20=off, 21=blink, 22=on)\r\n");
  cli_putstr("\r\n-- Info -----------------------------------------------------\r\n");
  cli_putstr("  s              Full system status\r\n");
  cli_putstr("  ?              This help\r\n");
  cli_putstr("\r\n");
}

static void cmd_status(void) {
  char buf[80];

  /* System */
  cli_putstr("--- System ------------------------------------------\r\n");
  snprintf(buf, sizeof(buf), "  State:      %s\r\n", interrupt_state_str(Interrupt_GetState()));
  cli_putstr(buf);
  snprintf(buf, sizeof(buf), "  E-Stop:     %s\r\n", on_off_str(EmergencyStop_IsActivated()));
  cli_putstr(buf);
  snprintf(buf, sizeof(buf), "  Btn Start:  %s\r\n", on_off_str(Buttons_Start_Pressed()));
  cli_putstr(buf);
  snprintf(buf, sizeof(buf), "  Btn Reset:  %s\r\n", on_off_str(Buttons_Reset_Pressed()));
  cli_putstr(buf);

  /* Limit switches */
  uint32_t lim = LimitSwitch_Activated();
  cli_putstr("  LimSwitch:  ");
  if (lim == LIM_NO_LIM) {
    cli_putstr("none\r\n");
  } else {
    if (lim & LIM_X_MIN) cli_putstr("X_MIN ");
    if (lim & LIM_X_MAX) cli_putstr("X_MAX ");
    if (lim & LIM_Y_MIN) cli_putstr("Y_MIN ");
    if (lim & LIM_Y_MAX) cli_putstr("Y_MAX ");
    cli_putstr("\r\n");
  }

  /* Actuators */
  cli_putstr("--- Actuators ----------------------------------------\r\n");
  snprintf(buf, sizeof(buf), "  XY-Move:    %s    pos=(%ld, %ld) steps\r\n",
           busy_str(StepGenerator_IsBusy()),
           (long)current_pos_steps_x, (long)current_pos_steps_y);
  cli_putstr(buf);
  snprintf(buf, sizeof(buf), "  Rotator:    %s\r\n", busy_str(Rotator_IsBusy()));
  cli_putstr(buf);
  snprintf(buf, sizeof(buf), "  Piston:     %s\r\n", busy_str(Piston_IsBusy()));
  cli_putstr(buf);
  snprintf(buf, sizeof(buf), "  Magnet:     %s\r\n", on_off_str(Magnet_GetState()));
  cli_putstr(buf);

  /* Status LEDs */
  cli_putstr("--- Status LEDs --------------------------------------\r\n");
  snprintf(buf, sizeof(buf), "  Green:      %s\r\n", led_type_str(StatusLed_Read(STATUSLED_GREEN)));
  cli_putstr(buf);
  snprintf(buf, sizeof(buf), "  Yellow:     %s\r\n", led_type_str(StatusLed_Read(STATUSLED_YELLOW)));
  cli_putstr(buf);
  snprintf(buf, sizeof(buf), "  Red:        %s\r\n", led_type_str(StatusLed_Read(STATUSLED_RED)));
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
  /* Reset local position tracker after homing */
  current_pos_steps_x = 0;
  current_pos_steps_y = 0;
  cli_ok();
}

static void cmd_move(const char* args) {
  int32_t x, y;
  if (sscanf(args, "%ld %ld", &x, &y) != 2) {
    cli_err("usage: m <x> <y>");
    return;
  }
  if (Interrupt_GetState() != IS_READY && Interrupt_GetState() != IS_RUNNING) {
    cli_err("not ready — home first (h)");
    return;
  }
  if (StepGenerator_IsBusy()) { cli_busy(); return; }

  static MoveBlock_t block;
  block = StepGenerator_GenerateBlock(x, y);
  if (!StepGenerator_StartMove(&block)) { cli_busy(); return; }

  cli_wait_step_generator();
  cli_ok();
}

static void cmd_move_planner(const char* args) {
  int32_t x_um, y_um;
  if (sscanf(args, "%ld %ld", &x_um, &y_um) != 2) {
    cli_err("usage: b <x> <y>  [um]");
    return;
  }
  if (Interrupt_GetState() != IS_READY && Interrupt_GetState() != IS_RUNNING) {
    cli_err("not ready — home first (h)");
    return;
  }
  if (StepGenerator_IsBusy()) { cli_busy(); return; }

  int32_t target_steps_x = (int32_t)((float)x_um * CONFIG_STEPS_PER_MM_X / 1000.0f);
  int32_t target_steps_y = (int32_t)((float)y_um * CONFIG_STEPS_PER_MM_Y / 1000.0f);
  int32_t delta_x = target_steps_x - current_pos_steps_x;
  int32_t delta_y = target_steps_y - current_pos_steps_y;
  current_pos_steps_x = target_steps_x;
  current_pos_steps_y = target_steps_y;

  static MoveBlock_t block;
  block = StepGenerator_GenerateBlock(delta_x, delta_y);
  if (!StepGenerator_StartMove(&block)) { cli_busy(); return; }

  cli_wait_step_generator();
  cli_ok();
}

static void cmd_rotate(const char* args) {
  int32_t steps;
  if (sscanf(args, "%ld", &steps) != 1) {
    cli_err("usage: r <steps>");
    return;
  }
  if (Rotator_IsBusy()) { cli_busy(); return; }

  static RotateBlock_t block;
  block = Rotator_GenerateBlock(steps);
  if (!Rotator_StartMove(&block)) { cli_busy(); return; }

  cli_wait_rotator();
  cli_ok();
}

static void cmd_piston(const char* args) {
  int val;
  if (sscanf(args, "%d", &val) != 1 || val < 0 || val >= (int)PISTON_POS_COUNT) {
    cli_err("usage: p <0..3>  (0=START 1=MOVE 2=GRAB 3=RELEASE)");
    return;
  }
  if (Piston_IsBusy()) { cli_busy(); return; }

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

static void cmd_led(const char* args) {
  int val;
  if (sscanf(args, "%d", &val) != 1 || (val != 0 && val != 1)) {
    cli_err("usage: l <0|1>");
    return;
  }
  Leds_Set((bool)val);
  cli_ok();
}

static void cmd_led_signal(const char* args) {
  int val;
  if (sscanf(args, "%d", &val) != 1 || val < 0 || val > 29) {
    cli_err("usage: a <0..29>  (0-9=green, 10-19=yellow, 20-29=red; x%3: 0=off 1=blink 2=on)");
    return;
  }

  StatusLeds_e led;
  int mode;
  if (val >= 20)      { led = STATUSLED_RED;    mode = val % 10 % 3; }
  else if (val >= 10) { led = STATUSLED_YELLOW;  mode = val % 10 % 3; }
  else                { led = STATUSLED_GREEN;   mode = val % 3; }

  switch (mode) {
    case 0: StatusLeds_Off(led);   break;
    case 1: StatusLeds_Blink(led); break;
    case 2: StatusLeds_On(led);    break;
    default: break;
  }
  cli_ok();
}

/* ========================
 *   STATE MACHINE TEST
 * ======================== */

static void cmd_run_sm(const char* args) {
  if (Interrupt_GetState() != IS_READY && Interrupt_GetState() != IS_RUNNING) {
    cli_err("not ready — home first (h), then retry");
    return;
  }
  if (StepGenerator_IsBusy() || Rotator_IsBusy() || Piston_IsBusy()) {
    cli_err("actuators busy — wait for current move to finish");
    return;
  }

  PuzzleCommand cmd = PuzzleCommand_init_zero;

  float px, py, plx, ply, rot = 0.0f;
  int n = sscanf(args, "%f %f %f %f %f", &px, &py, &plx, &ply, &rot);

  if (n >= 4) {
    /* --- Custom single-piece sequence --- */
    cmd.pieces_count  = 1;
    cmd.pieces[0].piece_id = 1;
    cmd.pieces[0].pick_x   = px;
    cmd.pieces[0].pick_y   = py;
    cmd.pieces[0].place_x  = plx;
    cmd.pieces[0].place_y  = ply;
    cmd.pieces[0].rotation = rot;

    char info[80];
    snprintf(info, sizeof(info),
             "SM test: 1 piece  pick=(%.1f,%.1f) place=(%.1f,%.1f) rot=%.1f deg\r\n",
             px, py, plx, ply, rot);
    cli_putstr(info);
  } else {
    /* --- Default 2-piece test sequence --- */
    cmd.pieces_count = 2;

    cmd.pieces[0].piece_id = 1;
    cmd.pieces[0].pick_x   = 30.0f;
    cmd.pieces[0].pick_y   = 30.0f;
    cmd.pieces[0].place_x  = 80.0f;
    cmd.pieces[0].place_y  = 30.0f;
    cmd.pieces[0].rotation =  0.0f;

    cmd.pieces[1].piece_id = 2;
    cmd.pieces[1].pick_x   =  50.0f;
    cmd.pieces[1].pick_y   =  60.0f;
    cmd.pieces[1].place_x  = 120.0f;
    cmd.pieces[1].place_y  =  80.0f;
    cmd.pieces[1].rotation =  90.0f;

    cli_putstr("SM test: 2 default pieces\r\n");
    cli_putstr("  piece 1: pick=(30,30) place=(80,30)   rot=0 deg\r\n");
    cli_putstr("  piece 2: pick=(50,60) place=(120,80)  rot=90 deg\r\n");
  }

  cli_putstr("Running... (Ctrl+C has no effect — wait for completion)\r\n");

  /* Warm up the state machine: SM_ESTOP → SM_WAIT_FOR_START when IS_READY */
  for (int i = 0; i < 10; i++) StateMachine_Update();

  /* Inject the command directly, bypassing the button / dispatcher path */
  StateMachine_StartManual(&cmd);

  /* Drive the state machine until all pieces are placed or an error occurs */
  uint32_t start     = HAL_GetTick();
  uint32_t last_tick = start;

  while (HAL_GetTick() - start < CLI_SM_TIMEOUT_MS) {
    StateMachine_Update();

    if (Interrupt_GetState() == IS_ESTOP) {
      cli_err("emergency stop triggered — sequence aborted");
      return;
    }

    if (StateMachine_IsDone()) {
      cli_putstr("\r\n");
      cli_ok();
      return;
    }

    /* Print a heartbeat dot every second so the terminal doesn't look frozen */
    if (HAL_GetTick() - last_tick >= 1000u) {
      cli_putstr(".");
      last_tick = HAL_GetTick();
    }
  }

  cli_putstr("\r\n");
  cli_err("state machine sequence timeout (>5 min)");
}

/* ========================
 *   PRIVATE FUNCTIONS — dispatcher
 * ======================== */

static void cli_dispatch(char* line) {
  while (*line == ' ') line++;
  if (*line == '\0') return;

  char  cmd  = *line;
  char* args = line + 1;
  while (*args == ' ') args++;

  switch (cmd) {
    case '?': cmd_help();               break;
    case 's': cmd_status();             break;
    case 'h': cmd_home();               break;
    case 'm': cmd_move(args);           break;
    case 'r': cmd_rotate(args);         break;
    case 'p': cmd_piston(args);         break;
    case 'g': cmd_magnet(args);         break;
    case 'l': cmd_led(args);            break;
    case 'b': cmd_move_planner(args);   break;
    case 'a': cmd_led_signal(args);     break;
    case 'x': cmd_run_sm(args);         break;
    default:
      cli_err("unknown command — type ? for help");
      break;
  }
}

/* ========================
 *   PUBLIC API
 * ======================== */

void TestCLI_Init(UART_HandleTypeDef* huart) {
  cli_huart = huart;

  /* Set up a local dispatcher so the state machine can send Ack messages
   * without crashing. Acks are binary protobuf blobs sent over the CLI
   * UART — a few garbled bytes may appear in the terminal when a sequence
   * finishes; this is expected and harmless. */
  UartReceiver_Init(&cli_sm_uart, huart);
  CommandDispatcher_Init(&cli_sm_dispatcher, &cli_sm_uart);
  StateMachine_Init(&cli_sm_dispatcher);
}

void TestCLI_Run(void) {
  char line[CLI_LINE_BUF_SIZE];

  cli_putstr("\r\nBOOT\r\n");
  cli_putstr("=== PuzzleSolver Test CLI ===\r\n");
  cli_putstr("Type ? for help\r\n\r\n");

  for (;;) {
    cli_putstr("> ");
    cli_readline(line, sizeof(line));
    cli_dispatch(line);
  }
}
