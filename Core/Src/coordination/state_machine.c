#include "main.h"
#include "state_machine.h"
#include "motion_planner.h"
#include "step_generator.h"

typedef enum {
  SM_IDLE,
  SM_CALC_TO_PICK,
  SM_MOVE_TO_PICK,
  SM_LOWER_TO_PICK,
  SM_GRAB_PIECE, 
  SM_LIFT_PIECE,
  SM_CALC_TO_PLACE,
  SM_MOVE_TO_PLACE,
  SM_ROTATE_PIECE,
  SM_LOWER_TO_PLACE,
  SM_RELEASE_PIECE, 
  SM_LIFT_EMPTY,
  SM_NEXT_PIECE
} State_e;

static State_e current_state;
static CommandDispatcher_t* sm_dispatcher;
static Piston_t* sm_piston;
static Magnet_t* sm_magnet;
static Rotator_t* sm_rotator;

static PuzzleCommand current_puzzle;
static uint8_t current_piece_idx;
static MoveBlock_t active_move;

void StateMachine_Init(CommandDispatcher_t* dispatcher, Piston_t* piston, Magnet_t* magnet, Rotator_t* rotator) {
  sm_dispatcher = dispatcher;
  sm_piston = piston;
  sm_magnet = magnet;
  sm_rotator = rotator;
  
  current_state = SM_IDLE;
  current_piece_idx = 0;

  MotionPlanner_Init();
}

void StateMachine_Update(void) {
  PieceCommand* piece;

  switch (current_state) {
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

    case SM_CALC_TO_PICK: {
      piece = &current_puzzle.pieces[current_piece_idx];
      active_move = MotionPlanner_PlanMoveToMM(piece->pick_x, piece->pick_y);
      StepGenerator_StartMove(&active_move);

      current_state = SM_MOVE_TO_PICK;
      break;
    }

    case SM_MOVE_TO_PICK:
      if (!StepGenerator_IsBusy()) {
        Piston_Set(sm_piston, PISTON_POS_GRAB);
        current_state = SM_LOWER_TO_PICK;
      }
      break;

    case SM_LOWER_TO_PICK:
      if (!piston_moving) {
        Magnet_SetState(sm_magnet, true); 
        current_state = SM_GRAB_PIECE;
      }
      break;

    case SM_GRAB_PIECE:
      Piston_Set(sm_piston, PISTON_POS_MOVE);
      current_state = SM_LIFT_PIECE;
      break;

    case SM_LIFT_PIECE:
      if (!piston_moving) {
        piece = &current_puzzle.pieces[current_piece_idx];
        active_move = MotionPlanner_PlanMoveToMM(piece->place_x, piece->place_y);
        StepGenerator_StartMove(&active_move);

        current_state = SM_MOVE_TO_PLACE;
      }
      break;

    case SM_MOVE_TO_PLACE:
      if (!StepGenerator_IsBusy()) {
        piece = &current_puzzle.pieces[current_piece_idx];
        Rotator_SetAngle(sm_rotator, piece->rotation);
        
        current_state = SM_ROTATE_PIECE;
      }
      break;

    case SM_ROTATE_PIECE:
      if (!Rotator_IsBusy(sm_rotator)) {
        Piston_Set(sm_piston, PISTON_POS_RELEASE);
        current_state = SM_LOWER_TO_PLACE;
      }
      break;

    case SM_LOWER_TO_PLACE:
      if (!piston_moving) {
        Magnet_SetState(sm_magnet, false);
        current_state = SM_RELEASE_PIECE;
      }
      break;

    case SM_RELEASE_PIECE:
      Piston_Set(sm_piston, PISTON_POS_MOVE);
      current_state = SM_LIFT_EMPTY;
      break;

    case SM_LIFT_EMPTY:
      if (!piston_moving) {
        piece = &current_puzzle.pieces[current_piece_idx];
        CommandDispatcher_SendAck(sm_dispatcher, Status_STATUS_OK, piece->piece_id);
        current_state = SM_NEXT_PIECE;
      }
      break;

    case SM_NEXT_PIECE:
      current_piece_idx++;

      if (current_piece_idx < current_puzzle.pieces_count) {
        current_state = SM_CALC_TO_PICK; 
      } else {
        CommandDispatcher_SendAck(sm_dispatcher, Status_STATUS_DONE, 0);
        current_state = SM_IDLE;
      }
      break;
  }
}