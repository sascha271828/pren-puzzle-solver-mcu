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
#include "status_leds.h"
#include "step_generator.h"
#include "sys_config.h"

/**
 * @brief Internal states for the puzzle coordination logic.
 */
typedef enum {
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
  SM_ESTOP,
  SM_ERROR
} State_e;

static State_e current_state;
static CommandDispatcher_t* sm_dispatcher;
static PuzzleCommand current_puzzle;
static uint8_t current_piece_idx;
static uint32_t wait_start_tick;

static PieceCommand* piece;
static MoveBlock_t active_xy_move;
static RotateBlock_t active_rot_move;

void StateMachine_Init(CommandDispatcher_t* dispatcher) {
  sm_dispatcher = dispatcher;
  current_state = SM_ESTOP;
  current_piece_idx = 0;
  Leds_Set(false);
}

void StateMachine_Update(void) {
  InterruptState_t is_state = Interrupt_GetState();

  /* check interrup state for emergency stop */
  if (is_state == IS_ESTOP) {
    current_state = SM_ESTOP;
  }

  switch (current_state) {
    /*-------------*/
    /* BASE STATES */
    /*-------------*/
    case SM_ESTOP:
      if (is_state == IS_READY) {
        current_state = SM_WAIT_FOR_START;
        Buttons_Start_RearmPressDetection();
        StatusLeds_On(STATUSLED_YELLOW);
      }
      break;

    case SM_WAIT_FOR_START:
      StatusLeds_Off(STATUSLED_GREEN);
      if (Buttons_Start_Pressed()) {
        StatusLeds_Blink(STATUSLED_YELLOW);
        Buttons_Start_RearmPressDetection();
        Leds_Set(true);
        CommandDispatcher_SendAck(sm_dispatcher, Status_STATUS_READY, 0);
        current_state = SM_IDLE;
      }
      break;

    /*------------------*/
    /* WAIT FOR RESULTS */
    /*------------------*/
    case SM_IDLE:
      if (CommandDispatcher_HasCommand(sm_dispatcher)) {
        Leds_Set(false);
        current_puzzle = *CommandDispatcher_GetCommand(sm_dispatcher);
        current_piece_idx = 0;
        if (current_puzzle.pieces_count > 0) {
          current_state = SM_CALC_TO_PICK;
          CommandDispatcher_SendAck(sm_dispatcher, Status_STATUS_BUSY, 0);
        }
      }
      break;

    /*--------------------*/
    /* CALCUAITON PICK UP */
    /*--------------------*/
    case SM_CALC_TO_PICK:
      piece = &current_puzzle.pieces[current_piece_idx];
      active_xy_move = MotionPlanner_PlanMoveToPickMM(piece->pick_x, piece->pick_y);
      StepGenerator_StartMove(&active_xy_move);
      current_state = SM_MOVE_TO_PICK;
      break;

    /*----------------------*/
    /* MOVE SEQUENCE PICK UP*/
    /*----------------------*/
    case SM_MOVE_TO_PICK:
      if (!StepGenerator_IsBusy()) {
        wait_start_tick = HAL_GetTick();
        current_state = SM_WAIT_BEFORE_LOWER_PICK;
      }
      break;

    case SM_WAIT_BEFORE_LOWER_PICK:
      if (HAL_GetTick() - wait_start_tick >= CONIFG_SM_WAIT_BEFORE_LOWER_PICK) {
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
      if (HAL_GetTick() - wait_start_tick >= CONIFG_SM_WAIT_BEFORE_PICK && !Rotator_IsBusy()) {
        Magnet_SetState(true);
        wait_start_tick = HAL_GetTick();
        current_state = SM_WAIT_MAGNET_ON;
      }
      break;

    case SM_WAIT_MAGNET_ON:
      if (HAL_GetTick() - wait_start_tick >= CONIFG_SM_WAIT_AFTER_PICK) {
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
      if (HAL_GetTick() - wait_start_tick >= CONIFG_SM_WAIT_AFTER_LIFT) {
        current_state = SM_CALC_TO_PLACE;
      }
      break;

    /*------------------*/
    /* CALCUAITON PLACE */
    /*------------------*/
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

    /*--------------------*/
    /* MOVE SEQUENCE PLACE*/
    /*--------------------*/
    case SM_MOVE_TO_PLACE:
      if (!StepGenerator_IsBusy() && !Rotator_IsBusy()) {
        wait_start_tick = HAL_GetTick();
        current_state = SM_WAIT_BEFORE_LOWER_PLACE;
      }
      break;

    case SM_WAIT_BEFORE_LOWER_PLACE:
      if (HAL_GetTick() - wait_start_tick >= CONIFG_SM_WAIT_BEFORE_LOWER_PLACE) {
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
      if (HAL_GetTick() - wait_start_tick >= CONIFG_SM_WAIT_BEFORE_RELEASE) {
        Magnet_SetState(false);
        wait_start_tick = HAL_GetTick();
        current_state = SM_WAIT_MAGNET_OFF;
      }
      break;

    case SM_WAIT_MAGNET_OFF:
      if (HAL_GetTick() - wait_start_tick >= CONIFG_SM_WAIT_AFTER_RELEASE) {
        current_state = SM_RELEASE_PIECE;
      }
      break;

    case SM_RELEASE_PIECE:
      Piston_Set(PISTON_POS_MOVE);
      current_state = SM_LIFT_EMPTY;
      Rotator_ReturnStart();
      break;

    case SM_LIFT_EMPTY:
      if (!Piston_IsBusy()) {
        if (current_piece_idx + 1 >= current_puzzle.pieces_count) {
          active_xy_move = MotionPlanner_PlanMoveToPickMM(0, 0);
          StepGenerator_StartMove(&active_xy_move);
        }
        current_state = SM_NEXT_PIECE;
      }
      break;

    /*------------*/
    /* NEXT PIECE */
    /*------------*/
    case SM_NEXT_PIECE:
      if (!StepGenerator_IsBusy()) {
        current_piece_idx++;
        if (current_piece_idx < current_puzzle.pieces_count) {
          current_state = SM_CALC_TO_PICK;
        } else {
          Rotator_ReturnStart();
          CommandDispatcher_SendAck(sm_dispatcher, Status_STATUS_DONE, 0);
          StatusLeds_On(STATUSLED_GREEN);
          current_state = SM_ESTOP;
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