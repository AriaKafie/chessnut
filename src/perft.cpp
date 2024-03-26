
#include "search.h"
#include "perft.h"
#include "position.h"
#include "debug.h"
#include "ui.h"
#include "util.h"

#include "movegen.h"
#include "moveordering.h"

#include <thread>
#include <chrono>

uint64_t leaves;

template<Color SideToMove>
void expand(int depth, int ply_from_root) {

  if (depth == 0) {
    leaves++;
    return;
  }

  state_stack[ply_from_root] = Position::st;

  if constexpr (SideToMove == WHITE) {
    MoveList<WHITE> ml(false);
    for (int i = 0; i < ml.length(); i++) {
      do_move<WHITE>(ml[i]);
      expand<BLACK>(depth - 1, ply_from_root + 1);
      undo_move<WHITE>(ml[i]);
      Position::st = state_stack[ply_from_root];
    }
  }
  else {
    MoveList<BLACK> ml(false);
    for (int i = 0; i < ml.length(); i++) {
      do_move<BLACK>(ml[i]);
      expand<WHITE>(depth - 1, ply_from_root + 1);
      undo_move<BLACK>(ml[i]);
      Position::st = state_stack[ply_from_root];
    }
  }

}

void Perft::go(int depth) {

  state_stack[0] = Position::st;

  if (Position::white_to_move()) {
    for (int plies_to_search = 1; plies_to_search <= depth; plies_to_search++) {
      leaves = 0;
      auto start = curr_time_millis();
      MoveList<WHITE> movelist(true);
      for (Move m : movelist) {
        do_move<WHITE>(m);
        expand<BLACK>(plies_to_search - 1, 1);
        undo_move<WHITE>(m);
        Position::st = state_stack[0];
      }
      auto delta = curr_time_millis() - start;
      std::cout << "\nDepth " << plies_to_search << ": " << leaves << " (" << delta << " ms)\n";
    }
  }
  else {
    for (int plies_to_search = 1; plies_to_search <= depth; plies_to_search++) {
      leaves = 0;
      auto start = curr_time_millis();
      MoveList<BLACK> movelist(true);
      for (Move m : movelist) {
        do_move<BLACK>(m);
        expand<WHITE>(plies_to_search - 1, 1);
        undo_move<BLACK>(m);
        Position::st = state_stack[0];
      }
      auto delta = curr_time_millis() - start;
      std::cout << "\nDepth " << plies_to_search << ": " << leaves << " (" << delta << " ms)\n";
    }
  }

}
