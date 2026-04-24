#include "main.h"

#include "state_machine.h"

#include "buttons.h"
#include "homer.h"
#include "interrupt.h"
#include "limit_switch.h"
#include "motion_planner.h"
#include "piston.h"
#include "rotator.h"
#include "step_generator.h"
#include "sys_config.h"

/**
 * @brief Internal states for the puzzle coordination logic.
 */
typedef enum {
  SM_INIT_HOMING, /* Start the homing sequence to find the physical zero point
                   */
  SM_HOMING_BUSY, /* Wait for endstops to be triggered and stop the motors */
  SM_WAIT_FOR_START, /* Wait for the physical start button to be pressed */
  SM_IDLE,          /* System is ready to receive puzzle commands from the PC */
  SM_CALC_TO_PICK,  /* Calculate the path and offsets for the pick location */
  SM_MOVE_TO_PICK,  /* Wait for XY-axes to reach the pick position */
  SM_LOWER_TO_PICK, /* Extend the piston towards the puzzle piece */
  SM_WAIT_MAGNET_ON, /* Delay to allow the magnetic field to build up */
  SM_GRAB_PIECE,     /* Command the piston to lift the grabbed piece */
  SM_LIFT_PIECE,     /* Wait for the piston to finish lifting the piece */
  SM_CALC_TO_PLACE, /* Calculate path and rotation for the target place location
                     */
  SM_MOVE_TO_PLACE, /* Move XY-axes and rotator simultaneously to the target */
  SM_LOWER_TO_PLACE,  /* Extend the piston to place the piece on the board */
  SM_WAIT_MAGNET_OFF, /* Delay to ensure the piece is fully released */
  SM_RELEASE_PIECE,   /* Retract the piston after releasing the magnet */
  SM_LIFT_EMPTY, /* Wait for the piston to reach move height without a piece */
  SM_NEXT_PIECE, /* Advance to the next piece or finish the puzzle command */
  SM_ERROR       /* System error state (e.g., collision, e-stop) */
} State_e;

static State_e current_state;
static CommandDispatcher_t* sm_dispatcher;
static Magnet_t* sm_magnet;
static PuzzleCommand current_puzzle;
static uint8_t current_piece_idx;
static uint32_t wait_start_tick;

void StateMachine_Init(CommandDispatcher_t* dispatcher, Magnet_t* magnet) {
  sm_dispatcher = dispatcher;
  sm_magnet = magnet;

  current_state = SM_INIT_HOMING;
  current_piece_idx = 0;
}

/**
 * @brief Non-blocking state machine update loop.
 */
void StateMachine_Update(void) {
  PieceCommand* piece;
  MoveBlock_t active_xy_move;
  RotateBlock_t active_rot_move;

  /* --- Global Error Monitoring --- */
  /* Abort on limit switch hit (collision) or emergency stop during active moves
   */
  if (current_state > SM_HOMING_BUSY && current_state != SM_ERROR &&
      current_state != SM_IDLE && current_state != SM_WAIT_FOR_START) {
    if (LimitSwitch_Activated() || Interrupt_GetState() == IS_ESTOP) {
      StepGenerator_Abort();
      Rotator_Abort();
      Magnet_SetState(sm_magnet, false);

      CommandDispatcher_SendAck(sm_dispatcher, Status_STATUS_ERROR, 0);
      current_state = SM_ERROR;
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
        /* Signal the PC that the system is mechanically ready */
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
      active_xy_move =
          MotionPlanner_PlanMoveToPickMM(piece->pick_x, piece->pick_y);
      StepGenerator_StartMove(&active_xy_move);
      current_state = SM_MOVE_TO_PICK;
      break;

    case SM_MOVE_TO_PICK:
      if (!StepGenerator_IsBusy()) {
        Piston_Set(PISTON_POS_GRAB);
        current_state = SM_LOWER_TO_PICK;
      }
      break;

    case SM_LOWER_TO_PICK:
      if (!Piston_IsBusy()) {
        Magnet_SetState(sm_magnet, true);
        wait_start_tick = HAL_GetTick();
        current_state = SM_WAIT_MAGNET_ON;
      }
      break;

    case SM_WAIT_MAGNET_ON:
      if (HAL_GetTick() - wait_start_tick >= CONFIG_MAGNET_DELAY_MS) {
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
        wait_start_tick = HAL_GetTick();
        current_state = SM_WAIT_MAGNET_OFF;
      }
      break;

    case SM_WAIT_MAGNET_OFF:
      if (HAL_GetTick() - wait_start_tick >= CONFIG_MAGNET_DELAY_MS) {
        current_state = SM_RELEASE_PIECE;
      }
      break;

    case SM_RELEASE_PIECE:
      Piston_Set(PISTON_POS_MOVE);
      current_state = SM_LIFT_EMPTY;
      break;

    case SM_LIFT_EMPTY:
      if (!Piston_IsBusy()) {
        /* Reset rotator to zero position for the next piece */
        Rotator_ReturnStart();
        current_state = SM_NEXT_PIECE;
      }
      break;

    case SM_NEXT_PIECE:
      if (!Rotator_IsBusy()) {
        current_piece_idx++;
        if (current_piece_idx < current_puzzle.pieces_count) {
          current_state = SM_CALC_TO_PICK;
        } else {
          /* Entire puzzle command completed */
          CommandDispatcher_SendAck(sm_dispatcher, Status_STATUS_DONE, 0);
          current_state = SM_IDLE;
        }
      }
      break;

    case SM_ERROR:
      /* Block until user intervention (start button pressed) and e-stop
       * released */
      if (Buttons_Start_Pressed()) {
        if (Interrupt_GetState() != IS_ESTOP) {
          Buttons_Start_RearmPressDetection();
          /* Force homing to recover the coordinate system */
          current_state = SM_INIT_HOMING;
        } else {
          Buttons_Start_RearmPressDetection();
        }
      }
      break;
  }

  /* --- Manual control for CLI testing --- */
  bool StateMachine_IsIdle(void) { return (current_state == SM_IDLE); }

  void StateMachine_StartManual(PuzzleCommand * cmd) {
    current_puzzle = *cmd;
    current_piece_idx = 0;
    if (current_puzzle.pieces_count > 0) {
      current_state = SM_CALC_TO_PICK;
    }
  }
}