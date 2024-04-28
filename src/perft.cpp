
#include "search.h"
#include "perft.h"
#include "position.h"
#include "debug.h"
#include "uci.h"
#include "ui.h"
#include "util.h"
#include "movegen.h"
#include "moveordering.h"

#include <thread>
#include <chrono>

template<Color>
void expand(int depth);

uint64_t nodes;

template<Color C>
void go_bench(int depth) {
  std::cout << "depth  nodes       ms      nodes\\second\n" << std::left;
  MoveList<C> moves;
  for (int d = 1; d <= depth; d++) {
    nodes = 0;
    auto start = curr_time_millis();
    for (Move m : moves) {
      do_move<C>(m);
      expand<!C>(d - 1);
      undo_move<C>(m);
    }
    auto delta = curr_time_millis() - start;
    std::cout << std::setw(7) << d << std::setw(12) << nodes << std::setw(8) << delta << std::max(0, int(double(nodes) / (double(delta) / 1000.0))) << "\n";
  }
  std::cout << "\n";
}

template<Color C>
void perft(int depth) {
  uint64_t total_nodes = 0;
  MoveList<C> moves;
  for (Move m : moves) {
    nodes = 0;
    std::cout << move_to_uci(m) << ": ";
    do_move<C>(m);
    expand<!C>(depth - 1);
    undo_move<C>(m);
    std::cout << nodes << "\n";
    total_nodes += nodes;
  }
  std::cout << "\nnodes searched: " << total_nodes << "\n\n";
}

template<Color SideToMove>
void expand(int depth) {

  if (depth == 0) {
    nodes++;
    return;
  }

  MoveList<SideToMove> moves;

  if (depth == 1) {
    nodes += moves.size();
    return;
  }

  for (Move m : moves) {
    do_move<SideToMove>(m);
    expand<!SideToMove>(depth - 1);
    undo_move<SideToMove>(m);
  }

}

void Perft::bench(int depth) {
  if (Position::white_to_move())
    go_bench<WHITE>(depth);
  else
    go_bench<BLACK>(depth);
}

void Perft::go(int depth) {
  if (Position::white_to_move())
    perft<WHITE>(depth);
  else
    perft<BLACK>(depth);
}
