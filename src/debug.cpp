
#include "debug.h"

#include <iomanip>
#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>

#include "movegen.h"
#include "uci.h"
#include "transpositiontable.h"
#include "util.h"

uint64_t perft_nodes;

template<Color SideToMove>
void expand(int depth)
{
    if (depth == 0)
    {
        perft_nodes++;
        return;
    }

    MoveList<SideToMove> moves;

    if (depth == 1)
    {
        perft_nodes += moves.size();
        return;
    }

    for (Move m : moves)
    {
        do_move<SideToMove>(m);
        expand<!SideToMove>(depth - 1);
        undo_move<SideToMove>(m);
    }
}

template<Color SideToMove>
void performance_test(int depth) {

    uint64_t total_nodes = 0;

    MoveList<SideToMove> moves;

    for (Move m : moves)
    {
        perft_nodes = 0;

        do_move<SideToMove>(m);
        expand<!SideToMove>(depth - 1);
        undo_move<SideToMove>(m);

        std::cout << move_to_uci(m) << ": " << perft_nodes << std::endl;

        total_nodes += perft_nodes;
    }

    std::cout << "\nnodes searched: " << total_nodes << "\n" << std::endl;
}

void Debug::perft(std::istringstream& ss) {

    int   depth;
    ss >> depth;

    if (Position::white_to_move()) performance_test<WHITE>(depth);
    else                           performance_test<BLACK>(depth);
}

extern RepInfo repetition_table[];

void Debug::go()
{
  std::cout << lsb(TT_SIZE) << "\n";
}

void Debug::gameinfo() {

  if (RepetitionTable::has_repeated()) {
    std::cout << "draw\n";
    return;
  }

  int count = 0;
  for (int i = 0; i < RT_SIZE; i++) {
    if (repetition_table[i].occurrences)
      count++;
    if (count >= 100) {
      std::cout << "draw\n";
      return;
    }
  }

  if (Position::white_to_move()) {
    MoveList<WHITE> moves;
    if (moves.size())
      std::cout << "nonterminal\n";
    else if (moves.in_check())
      std::cout << "mate\n";
    else
      std::cout << "draw\n";
  } else {
    MoveList<BLACK> moves;
    if (moves.size())
      std::cout << "nonterminal\n";
    else if (moves.in_check())
      std::cout << "mate\n";
    else
      std::cout << "draw\n";
  }
}
