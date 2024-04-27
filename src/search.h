
#ifndef SEARCH_H
#define SEARCH_H

#include "types.h"

//struct PlyInfo {
//  Move move_played;
//};
//
//PlyInfo search_stack[MAX_PLY];


/*
  if (quietbetacut)
    counter_moves[from_to(search_stack[ply_from_root - 1].move_played)] = move;

  for (Move& m : *this)
    if (m == counter_moves[from_to(search_stack[ply_from_root - 1].move_played)])
      m += counter_move_score (* depth?);
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

} // namespace Search

#endif
