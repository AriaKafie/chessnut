
#include "debug.h"

#include <iomanip>
#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <vector>

#include "movegen.h"
#include "moveordering.h"
#include "position.h"
#include "search.h"
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

    uint64_t total_ms = 0, total_nodes = 0;

    MoveList<SideToMove> moves;

    for (Move m : moves)
    {
        nodes = 0;

        auto start = curr_time_millis();

        do_move<SideToMove>(m);
        search<!SideToMove>(depth - 1);
        undo_move<SideToMove>(m);

        auto end = curr_time_millis();

        total_ms += end - start;

        std::cout << move_to_uci(m) << ": " << nodes << std::endl;

        total_nodes += nodes;
    }

    std::cout << "\nnodes searched: " << total_nodes << "\nin " << total_ms << " ms\n" << std::endl;
}

void Debug::perft(std::istringstream& ss) {

    int   depth;
    ss >> depth;

    if (Position::white_to_move()) performance_test<WHITE>(depth);
    else                           performance_test<BLACK>(depth);
}

void Debug::go() {

    char buffer[4];

    char c, *ptr = buffer;

    for (int i = 0; i < 4; i++)
    {
        std::cin >> c;
        *ptr++ = c;
    }
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
