
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

template<Color>
void expand(int depth);

uint64_t leaves;

template<Color C>
void run(int depth) {
  for (int plies_to_search = 1; plies_to_search <= depth; plies_to_search++) {
    leaves = 0;
    auto start = curr_time_millis();
    MoveList<C> moves;
    for (Move m : moves) {
      do_move<C>(m);
      expand<!C>(plies_to_search - 1);
      undo_move<C>(m);
    }
    auto delta = curr_time_millis() - start;
    std::cout << "\nDepth " << plies_to_search << ": " << leaves << " (" << delta << " ms)\n";
  }
}

template<Color SideToMove>
void expand(int depth) {

  if (depth == 0) {
    leaves++;
    return;
  }

  MoveList<SideToMove> moves;

  if (depth == 1) {
    leaves += moves.length();
    return;
  }

  for (Move m : moves) {
    do_move<SideToMove>(m);
    expand<!SideToMove>(depth - 1);
    undo_move<SideToMove>(m);
  }

}

void Perft::go(int depth) {
  if (Position::white_to_move())
    run<WHITE>(depth);
  else
    run<BLACK>(depth);
}
