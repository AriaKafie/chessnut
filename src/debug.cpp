
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

    uint64_t total_ms = 0, total_nodes = 0;

    MoveList<SideToMove> moves;

    for (Move m : moves)
    {
        perft_nodes = 0;

        auto start = curr_time_millis();

        do_move<SideToMove>(m);
        search<!SideToMove>(depth - 1);
        undo_move<SideToMove>(m);

        auto end = curr_time_millis();

        total_ms += end - start;

        std::cout << move_to_uci(m) << ": " << perft_nodes << std::endl;

        total_nodes += perft_nodes;
    }

    std::cout << "\nnodes searched: " << total_nodes << "\nin " << total_ms << " ms\n" << std::endl;
}

void Debug::perft(std::istringstream& ss) {

    int   depth;
    ss >> depth;

    if (Position::white_to_move()) performance_test<WHITE>(depth);
    else                           performance_test<BLACK>(depth);
}

extern RepInfo repetition_table[];

extern int reductions[MAX_PLY][128];

void Debug::go() {

    for (Piece p = 0; p <= B_KING; p++)
    {
        Bitboard b = bitboards[p];

        std::cout << to_string(_blsi_u64(b));
    }
}

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
