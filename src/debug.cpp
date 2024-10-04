
#include "debug.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <vector>

#include "movegen.h"
#include "moveordering.h"
#include "position.h"
#include "transpositiontable.h"
#include "uci.h"

static uint64_t nodes;

template<Color SideToMove>
void search(int depth)
{
    if (depth == 0)
    {
        nodes++;
        return;
    }

    MoveList<SideToMove> moves;

    if (depth == 1)
    {
        nodes += moves.size();
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

    uint64_t elapsed = 0, total_nodes = 0;

    MoveList<SideToMove> moves;

    for (Move m : moves)
    {
        nodes = 0;

        auto start = std::chrono::steady_clock::now();

        do_move<SideToMove>(m);
        search<!SideToMove>(depth - 1);
        undo_move<SideToMove>(m);

        auto end = std::chrono::steady_clock::now();

        elapsed += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        std::cout << move_to_uci(m) << ": " << nodes << std::endl;

        total_nodes += nodes;
    }

    std::cout << "\nnodes searched: " << total_nodes << "\nin " << (elapsed / 1000) << " ms\n" << std::endl;
}

void Debug::perft(std::istringstream& ss) {

    int   depth;
    ss >> depth;

    if (Position::white_to_move()) performance_test<WHITE>(depth);
    else                           performance_test<BLACK>(depth);
}

void Debug::go() {

    std::cout << to_string(0);
}

extern RepInfo repetition_table[];

void Debug::gameinfo() {

    if (RepetitionTable::draw())
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
        else                       std::cout << "draw"        << std::endl;
    }
}
