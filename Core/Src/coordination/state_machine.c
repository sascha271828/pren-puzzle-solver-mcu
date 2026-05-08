#include "main.h"
#include "state_machine.h"
#include "buttons.h"
#include "homer.h"
#include "interrupt.h"
#include "leds.h"
#include "limit_switch.h"
#include "motion_planner.h"
#include "piston.h"
#include "rotator.h"
#include "step_generator.h"
#include "sys_config.h"

#define DELAY_SETTLE_MS      400  
#define DELAY_MAGNET_PICK_MS 500  
#define DELAY_MAGNET_DROP_MS 300  

/**
 * @brief Internal states for the puzzle coordination logic.
 */
typedef enum {
  SM_INIT_HOMING,
  SM_HOMING_BUSY,
  SM_WAIT_FOR_START,
  SM_IDLE,
  SM_CALC_TO_PICK,
  SM_MOVE_TO_PICK,
  SM_WAIT_BEFORE_LOWER_PICK, 
  SM_LOWER_TO_PICK,
  SM_WAIT_ON_PIECE,        
  SM_WAIT_MAGNET_ON,
  SM_GRAB_PIECE,
  SM_LIFT_PIECE,
  SM_WAIT_AFTER_LIFT,       
  SM_CALC_TO_PLACE,
  SM_MOVE_TO_PLACE,
  SM_WAIT_BEFORE_LOWER_PLACE,
  SM_LOWER_TO_PLACE,
  SM_WAIT_BEFORE_RELEASE,   
  SM_WAIT_MAGNET_OFF,
  SM_RELEASE_PIECE,
  SM_LIFT_EMPTY,
  SM_NEXT_PIECE,
  SM_ERROR
} State_e;

static State_e current_state;
static CommandDispatcher_t* sm_dispatcher;
static PuzzleCommand current_puzzle;
static uint8_t current_piece_idx;
static uint32_t wait_start_tick;

static uint32_t led_turn_on_tick = 0;
static bool led_is_active = false;
#define CONFIG_LED_ON_TIME_MS 4000

static PieceCommand* piece;
static MoveBlock_t active_xy_move;
static RotateBlock_t active_rot_move;

void StateMachine_Init(CommandDispatcher_t* dispatcher) {
  sm_dispatcher = dispatcher;
  current_state = SM_INIT_HOMING;
  current_piece_idx = 0;
  Leds_Set(false);
  led_is_active = false;
}

void StateMachine_Update(void) {
  /* LED Timer Logic */
  if (led_is_active) {
    if (HAL_GetTick() - led_turn_on_tick >= CONFIG_LED_ON_TIME_MS) {
      Leds_Set(false);
      led_is_active = false;
    }
  }

  switch (current_state) {
    case SM_INIT_HOMING:
      Homer_HomingStart();
      current_state = SM_HOMING_BUSY;
      break;

    case SM_HOMING_BUSY:
      if (!Homer_IsRunning()) {
        MotionPlanner_Init();
        current_state = SM_WAIT_FOR_START;
      }
      break;

    case SM_WAIT_FOR_START:
      if (Buttons_Start_Pressed()) {
        Buttons_Start_RearmPressDetection();
        Leds_Set(true);
        led_is_active = true;
        led_turn_on_tick = HAL_GetTick();
        CommandDispatcher_SendAck(sm_dispatcher, Status_STATUS_READY, 0);
        current_state = SM_IDLE;
      }
      break;

    case SM_IDLE:
      if (CommandDispatcher_HasCommand(sm_dispatcher)) {
        current_puzzle = *CommandDispatcher_GetCommand(sm_dispatcher);
        current_piece_idx = 0;
        if (current_puzzle.pieces_count > 0) {
          current_state = SM_CALC_TO_PICK;
          CommandDispatcher_SendAck(sm_dispatcher, Status_STATUS_BUSY, 0);
        }
      }
      break;


    case SM_CALC_TO_PICK:
      piece = &current_puzzle.pieces[current_piece_idx];
      active_xy_move = MotionPlanner_PlanMoveToPickMM(piece->pick_x, piece->pick_y);
      StepGenerator_StartMove(&active_xy_move);
      current_state = SM_MOVE_TO_PICK;
      break;

    case SM_MOVE_TO_PICK:
      if (!StepGenerator_IsBusy()) {
        wait_start_tick = HAL_GetTick();
        current_state = SM_WAIT_BEFORE_LOWER_PICK;
      }
      break;

    case SM_WAIT_BEFORE_LOWER_PICK:
      if (HAL_GetTick() - wait_start_tick >= DELAY_SETTLE_MS) {
        Piston_Set(PISTON_POS_GRAB);
        current_state = SM_LOWER_TO_PICK;
      }
      break;

    case SM_LOWER_TO_PICK:
      if (!Piston_IsBusy()) {
        wait_start_tick = HAL_GetTick();
        current_state = SM_WAIT_ON_PIECE;
      }
      break;

    case SM_WAIT_ON_PIECE:
      if (HAL_GetTick() - wait_start_tick >= 200) {
        Magnet_SetState(true);
        wait_start_tick = HAL_GetTick();
        current_state = SM_WAIT_MAGNET_ON;
      }
      break;

    case SM_WAIT_MAGNET_ON:
      if (HAL_GetTick() - wait_start_tick >= DELAY_MAGNET_PICK_MS) {
        current_state = SM_GRAB_PIECE;
      }
      break;

    case SM_GRAB_PIECE:
      Piston_Set(PISTON_POS_MOVE);
      current_state = SM_LIFT_PIECE;
      break;

    case SM_LIFT_PIECE:
      if (!Piston_IsBusy()) {
        wait_start_tick = HAL_GetTick();
        current_state = SM_WAIT_AFTER_LIFT;
      }
      break;

    case SM_WAIT_AFTER_LIFT:
      if (HAL_GetTick() - wait_start_tick >= DELAY_SETTLE_MS) {
        current_state = SM_CALC_TO_PLACE;
      }
      break;

    /* --- PLACE SEQUENZ --- */

    case SM_CALC_TO_PLACE:
      piece = &current_puzzle.pieces[current_piece_idx];
      active_xy_move = MotionPlanner_PlanMoveToPlaceMM(piece->place_x, piece->place_y);
      StepGenerator_StartMove(&active_xy_move);

      int32_t rot_steps = (int32_t)(piece->rotation * 10.0f) * CONFIG_STEPS_PER_01_DEGREE;
      if (rot_steps != 0) {
        active_rot_move = Rotator_GenerateBlock(rot_steps);
        Rotator_StartMove(&active_rot_move);
      }
      current_state = SM_MOVE_TO_PLACE;
      break;

    case SM_MOVE_TO_PLACE:
      if (!StepGenerator_IsBusy() && !Rotator_IsBusy()) {
        wait_start_tick = HAL_GetTick();
        current_state = SM_WAIT_BEFORE_LOWER_PLACE;
      }
      break;

    case SM_WAIT_BEFORE_LOWER_PLACE:
      if (HAL_GetTick() - wait_start_tick >= DELAY_SETTLE_MS) {
        Piston_Set(PISTON_POS_RELEASE);
        current_state = SM_LOWER_TO_PLACE;
      }
      break;

    case SM_LOWER_TO_PLACE:
      if (!Piston_IsBusy()) {
        wait_start_tick = HAL_GetTick();
        current_state = SM_WAIT_BEFORE_RELEASE;
      }
      break;

    case SM_WAIT_BEFORE_RELEASE:
      if (HAL_GetTick() - wait_start_tick >= 200) {
        Magnet_SetState(false);
        wait_start_tick = HAL_GetTick();
        current_state = SM_WAIT_MAGNET_OFF;
      }
      break;

    case SM_WAIT_MAGNET_OFF:
      if (HAL_GetTick() - wait_start_tick >= DELAY_MAGNET_DROP_MS) {
        current_state = SM_RELEASE_PIECE;
      }
      break;

    case SM_RELEASE_PIECE:
      Piston_Set(PISTON_POS_MOVE);
      current_state = SM_LIFT_EMPTY;
      break;

    case SM_LIFT_EMPTY:
      if (!Piston_IsBusy()) {
        Rotator_ReturnStart();
        if (current_piece_idx + 1 >= current_puzzle.pieces_count) {
          active_xy_move = MotionPlanner_PlanMoveToPickMM(0, 0);
          StepGenerator_StartMove(&active_xy_move);
        }
        current_state = SM_NEXT_PIECE;
      }
      break;

    case SM_NEXT_PIECE:
      if (!Rotator_IsBusy() && !StepGenerator_IsBusy()) {
        current_piece_idx++;
        if (current_piece_idx < current_puzzle.pieces_count) {
          current_state = SM_CALC_TO_PICK;
        } else {
          CommandDispatcher_SendAck(sm_dispatcher, Status_STATUS_DONE, 0);
          current_state = SM_WAIT_FOR_START;
        }
      }
      break;

    case SM_ERROR:
      break;
  }
}

bool StateMachine_IsIdle(void) { return (current_state == SM_IDLE); }

void StateMachine_StartManual(PuzzleCommand* cmd) {
  current_puzzle = *cmd;
  current_piece_idx = 0;
  if (current_puzzle.pieces_count > 0) {
    current_state = SM_CALC_TO_PICK;
  }
}