
#include "movegen.h"
#include "uci.h"
#include "ui.h"
#include "search.h"
#include "transpositiontable.h"

#include <sstream>
#include <iomanip>
#include <iostream>
#include <fstream>
// 4q2r/2pk4/Q1n1bp1p/6p1/8/2NP4/PrPB1PPP/R5K1 w - - 0 1

extern RepInfo rep_table[];

namespace Debug {

  void go() {
    for (int i = 0; i < (1 << 18); i++) {
      RepInfo& ri = rep_table[i];
      if (ri.occurrences)
        std::cout << std::hex << ri.key << ": " << std::dec << (int)ri.occurrences << "\n";
    }
    std::cout << "ok\n";
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

  if (RepetitionTable::has_repeated()) {
    std::cout << "draw\n";
    return;
  }

  if (Position::white_to_move()) {
    MoveList<WHITE> moves;
    if (moves.size())
      std::cout << "nonterminal\n";
    else if (moves.incheck())
      std::cout << "mate\n";
    else
      std::cout << "draw\n";
  } else {
    MoveList<BLACK> moves;
    if (moves.size())
      std::cout << "nonterminal\n";
    else if (moves.incheck())
      std::cout << "mate\n";
    else
      std::cout << "draw\n";
  }
}
