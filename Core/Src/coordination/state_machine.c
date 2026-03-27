#include "state_machine.h"

#include "motion_planner.h"
#include "piston.h" 
#include "rotation.h" 
#include "step_generator.h"
#include "sys_config.h"

typedef enum {
  SM_IDLE,
  SM_CALC_TO_PICK,
  SM_MOVE_TO_PICK,
  SM_LOWER_TO_PICK,
  SM_GRAB_PIECE,
  SM_LIFT_PIECE,
  SM_CALC_TO_PLACE,
  SM_MOVE_TO_PLACE,
  SM_LOWER_TO_PLACE,
  SM_RELEASE_PIECE,
  SM_LIFT_EMPTY,
  SM_NEXT_PIECE
} State_e;

static State_e current_state;

static CommandDispatcher_t* sm_dispatcher;
static Magnet_t* sm_magnet;

static PuzzleCommand current_puzzle;
static uint8_t current_piece_idx;

static MoveBlock_t active_xy_move;
static RotateBlock_t active_rot_move;

void StateMachine_Init(CommandDispatcher_t* dispatcher,
                       Magnet_t* magnet) {
  sm_dispatcher = dispatcher;
  sm_magnet = magnet;

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

    case SM_CALC_TO_PICK:
      piece = &current_puzzle.pieces[current_piece_idx];

      active_xy_move =
          MotionPlanner_PlanMoveToPickMM(piece->pick_x, piece->pick_y);
      StepGenerator_StartMove(&active_xy_move);

      current_state = SM_MOVE_TO_PICK;
      break;

    case SM_MOVE_TO_PICK:
      if (!StepGenerator_IsBusy() && !Rotator_IsBusy()) {
        Piston_Set(PISTON_POS_GRAB);
        current_state = SM_LOWER_TO_PICK;
      }
      break;

    case SM_LOWER_TO_PICK:
      if (!Piston_IsBusy()) {
        Magnet_SetState(sm_magnet, true);
        current_state = SM_GRAB_PIECE;
      }
      break;

    case SM_GRAB_PIECE:
      Piston_Set(PISTON_POS_MOVE);
      current_state = SM_LIFT_PIECE;
      break;

    case SM_LIFT_PIECE:
      if (!Piston_IsBusy()) {
        current_state = SM_CALC_TO_PLACE;
      }
      break;

    case SM_CALC_TO_PLACE:
      piece = &current_puzzle.pieces[current_piece_idx];

      active_xy_move =
          MotionPlanner_PlanMoveToPlaceMM(piece->place_x, piece->place_y);
      StepGenerator_StartMove(&active_xy_move);

      int32_t rot_steps =
          (int32_t)(piece->rotation * 10.0f) * CONFIG_STEPS_PER_01_DEGREE;

      if (rot_steps != 0) {
        active_rot_move = Rotator_GenerateBlock(rot_steps);
        Rotator_StartMove(&active_rot_move);
      }

      current_state = SM_MOVE_TO_PLACE;
      break;

    case SM_MOVE_TO_PLACE:
      if (!StepGenerator_IsBusy() && !Rotator_IsBusy()) {
        Piston_Set(PISTON_POS_RELEASE);
        current_state = SM_LOWER_TO_PLACE;
      }
      break;

    case SM_LOWER_TO_PLACE:
      if (!Piston_IsBusy()) {
        Magnet_SetState(sm_magnet, false);
        current_state = SM_RELEASE_PIECE;
      }
      break;

    case SM_RELEASE_PIECE:
      Piston_Set(PISTON_POS_MOVE);
      current_state = SM_LIFT_EMPTY;
      break;

    case SM_LIFT_EMPTY:
      if (!Piston_IsBusy()) {
        piece = &current_puzzle.pieces[current_piece_idx];
        CommandDispatcher_SendAck(
            sm_dispatcher, Status_STATUS_OK, piece->piece_id);

        Rotator_ReturnStart();

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