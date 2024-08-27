
#include "perft.h"

#include <chrono>
#include <iomanip>

#include "debug.h"
#include "movegen.h"
#include "moveordering.h"
#include "position.h"
#include "search.h"
#include "uci.h"
#include "util.h"

template<Color>
void expand(int depth);

uint64_t pnodes;

template<Color C>
void go_bench(int depth) {

    std::cout << "depth  nodes       ms      nodes\\second\n" << std::left;

    MoveList<C> moves;

    for (int d = 1; d <= depth; d++)
    {
        pnodes = 0;
        auto start = curr_time_millis();
        for (Move m : moves)
        {
            do_move<C>(m);
            expand<!C>(d - 1);
            undo_move<C>(m);
        }

        auto delta = curr_time_millis() - start;
        std::cout << std::setw(7) << d << std::setw(12) << pnodes << std::setw(8) << delta << std::max(0, int(double(pnodes) / (double(delta) / 1000.0))) << "\n";
    }

    std::cout << "\n";
}

template<Color C>
void perft(int depth) {

    uint64_t total_nodes = 0;

    MoveList<C> moves;

    for (Move m : moves)
    {
        pnodes = 0;
        std::cout << move_to_uci(m) << ": ";
        do_move<C>(m);
        expand<!C>(depth - 1);
        undo_move<C>(m);
        std::cout << pnodes << "\n";
        total_nodes += pnodes;
    }

    std::cout << "\nnodes searched: " << total_nodes << "\n\n";
}

template<Color SideToMove>
void expand(int depth)
{
    if (depth == 0)
    {
        pnodes++;
        return;
    }

    MoveList<SideToMove> moves;

    if (depth == 1)
    {
        pnodes += moves.size();
        return;
    }

    for (Move m : moves)
    {
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
    if (Position::white_to_move()) perft<WHITE>(depth);
    else                           perft<BLACK>(depth);
}
