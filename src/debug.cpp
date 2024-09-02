
#include "debug.h"

#include <iomanip>
#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>

#include "movegen.h"
#include "position.h"
#include "transpositiontable.h"
#include "uci.h"

uint64_t perft_nodes;

template<Color SideToMove>
void search(int depth)
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
        search<!SideToMove>(depth - 1);
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
        search<!SideToMove>(depth - 1);
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

void Debug::go() {

    for (int i = 0; i < B_KING + 1; i++)
    {
        std::cout << to_string(bitboards[i]) << "\n";
    }
}

void Debug::gameinfo() {

  if (RepetitionTable::has_repeated())
  {
    std::cout << "draw" << std::endl;
    return;
  }

  for (int i = 0, count = 0; i < RT_SIZE; i++)
  {
    const RepInfo& ri = repetition_table[i];
  
    if (ri.occurrences)
      count += ri.occurrences;

    if (count >= 100)
    {
      std::cout << "draw" << std::endl;
      return;
    }
  }

  if (Position::white_to_move())
  {
    MoveList<WHITE> moves;

    if      (moves.size())     std::cout << "nonterminal" << std::endl;
    else if (moves.in_check()) std::cout << "mate"        << std::endl;
    else                       std::cout << "draw"        << std::endl;
  }
  else
  {
    MoveList<BLACK> moves;

    if      (moves.size())     std::cout << "nonterminal" << std::endl;
    else if (moves.in_check()) std::cout << "mate"        << std::endl;
    else                       std::cout << "draw" << std::endl;
  }
}
