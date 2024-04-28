
#ifndef SEARCH_H
#define SEARCH_H

#include "types.h"
#include "util.h"

//struct PlyInfo {
//  Killer killers;
//  Move current_move;
//  int refutation_history[SQUARE_NB * SQUARE_NB][B_KING + 1][SQUARE_NB];
//};
//
//PlyInfo search_stack[MAX_PLY];

/* = -search(search_stack + 1)

template<Color SideToMove>
int search(int depth, int alpha, int beta, PlyInfo* ply_ptr)
{
  if (null_ok)
  {
    ply_ptr->current_move = NULLMOVE;

    state_ptr->key ^= Zobrist::hash;
    eval = -search<!SideToMove>(depth - 3, -beta, -beta + 1, ply_ptr + 1);
  }

  MoveList moves = gen_moves<SideToMove>();

  for (Move m : moves)
  {
    ply_ptr->current_move = from_to(m);
    do_move<SideToMove>(m);
    eval = -search<!SideToMove>(depth - 1, -beta, -alpha, ply_ptr + 1);
    undo_move<SideToMove>(m);
  }
}

*/


/*
  if (quietbetacut)
  {
    ply_ptr->killers.add(moves[i] & 0xffff);

    Square   to   = to_sq(moves[i]);
    Piece    pc   = piece_on(from_sq(moves[i]));
    PlyInfo* prev = ply_ptr - 1;

    prev->refutation_history[from_to(prev->current_move)][pc][to] += 1 << depth;
  }
*/

inline constexpr int reduction[MAX_PLY] =
{
  0,1,1,1,1,2,2,2,2,2,
  2,2,2,2,2,3,3,3,3,3,
  3,3,3,3,3,3,3,3,3,3,
  3,3,3,3,3,3,3,3,3,3,
  4,4,4,4,4,4,4,4,4,4,
  5,5,5,5,5,5,5,5,5,5,
  6,6,6,6,6,6,6,6,6,6,
  7,7,7,7,7,7,7,7,7,7,
  8,8,8,8,8,8,8,8,8,8,
};

inline constexpr int depth_reduction[MAX_PLY] =
{
  0,0,0,0,0,0,0,0,0,0,
  1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,2,2,2,2,2,2,
  3,3,3,3,3,3,3,3,3,3,
  4,4,4,4,4,4,4,4,4,4,
  5,5,5,5,5,5,5,5,5,5,
  6,6,6,6,6,6,6,6,6,6,
  7,7,7,7,7,7,7,7,7,7,
  8,8,8,8,8,8,8,8,8,8,
};

constexpr int matescore = 100000;

namespace Search {

Move bestmove(uint64_t thinktime);
void init();
void go_infinite();

} // namespace Search

inline bool search_cancelled;

inline void start_timer(uint64_t thinktime) {
  auto start_time = curr_time_millis();
  while (true) {
    if (curr_time_millis() - start_time > thinktime) {
      search_cancelled = true;
      return;
    }
  }
}

inline void await_stop() {
  std::string in;
  do
  {
    std::getline(std::cin, in);
  } while (in != "stop");
  search_cancelled = true;
}

#endif
