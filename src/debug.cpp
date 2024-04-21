
#include "movegen.h"
#include "uci.h"
#include "ui.h"
#include "search.h"

#include <sstream>
#include <iomanip>
#include <iostream>
#include <fstream>
// 4q2r/2pk4/Q1n1bp1p/6p1/8/2NP4/PrPB1PPP/R5K1 w - - 0 1
namespace Debug {

  void go() {
    for (Square s = H1; s <= A8; s++) {
      std::cout << square_to_uci(s) << "\n";
    }
  }

}

std::string brd() {

  std::string board = Position::to_string();

  for (char& c : board)
    if (c == '\n')
      c = '?';

  return board;
}

void gameinfo() {

#define GAMEINFO(color)             \
  do {                              \
    MoveList<color> moves;          \
    if (moves.size())               \
      std::cout << "nonterminal\n"; \
    else if (moves.incheck())       \
      std::cout << "mate\n";        \
    else                            \
      std::cout << "draw\n";        \
  } while (false)

  if (Position::white_to_move()) { GAMEINFO(WHITE); } 
  else                           { GAMEINFO(BLACK); }

#undef GAMEINFO
}
