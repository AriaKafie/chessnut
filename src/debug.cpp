
#include "debug.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <vector>
#include <random>
#include <thread>

#include <intrin.h>
#include <immintrin.h>

#include "movegen.h"
#include "moveordering.h"
#include "position.h"
#include "transpositiontable.h"
#include "search.h"
#include "uci.h"

template<Color STM>
static std::string pv()
{
    if (RepetitionTable::draw())
        return "";

    std::string line;
    Move best = TranspositionTable::lookup_move();

    if (best == NO_MOVE)
        return "";

    bool legal = false;

    MoveList<STM> moves;
    for (Move m : moves)
        if (m == best) legal = true;

    if (!legal) return "";

    else
    {
        do_move<STM>(best);
        line = move_to_uci(best) + " " + pv<!STM>();
        undo_move<STM>(best);
    }

    return line;
}

std::string Debug::pv() {
    return Position::white_to_move() ? ::pv<WHITE>()
                                     : ::pv<BLACK>();
}

template<bool Root, Color SideToMove>
uint64_t PerfT(int depth)
{
    if (depth == 0)
        return 1;

    MoveList<SideToMove> moves;

    if (depth == 1 && !Root)
        return moves.size();

    uint64_t count, nodes = 0;
    
    for (Move m : moves)
    {
        do_move<SideToMove>(m);
        count = PerfT<false, !SideToMove>(depth - 1);
        undo_move<SideToMove>(m);

        nodes += count;

        if (Root)
            std::cout << move_to_uci(m) << ": " << count << std::endl;
    }

    return nodes;
}

void Debug::perft(std::istringstream& is)
{
    if (int depth; is >> depth)
    {
        auto start = std::chrono::steady_clock::now();
        uint64_t result = Position::white_to_move() ? PerfT<true, WHITE>(depth)
                                                    : PerfT<true, BLACK>(depth);
        auto end   = std::chrono::steady_clock::now();

        std::cout << "\nnodes searched: " << result << "\nin "
                  << (std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000) << " ms\n" << std::endl;
    }
    else
    {
        std::string        line, token;
        std::istringstream suite(perft_suite);
        bool               failed = false;

        while (std::getline(suite, line))
        {
            Position::set(line.substr(0, line.find(';')));

            std::istringstream is(line.substr(line.find(';')));

            for (uint64_t depth = is.str()[2] - '0', expected; is >> token >> expected; depth++)
            {
                std::cout << "Perft " << depth << " " << Position::fen() << std::endl;

                uint64_t result = Position::white_to_move() ? PerfT<false, WHITE>(depth)
                                                            : PerfT<false, BLACK>(depth);

                if (result != expected)
                {
                    failed = true;
                    std::cout << "ERROR\n" << std::endl;
                    break;
                }

                if (is.eof())
                    std::cout << "OK\n" << std::endl;
            }
        }

        std::cout << (failed ? "FAILED\n" : "ALL OK\n") << std::endl;
    }
}

std::string rep_table_to_string()
{
    extern RTEntry repetition_table[];

    std::stringstream ss;
    std::string s = "+------------------+------+----+\n";

    ss << s << "| key              | loc  |  # |\n" << s;

    for (int i = 0; i < RT_SIZE; i++)
        if (RTEntry& ri = repetition_table[i]; ri.occurrences)
        {
            ss << "| " << std::setw(16) << std::setfill('0') << std::hex << std::uppercase << ri.key;
            ss << " | " << std::setw(4) << std::setfill('0') << std::hex << std::uppercase << i;
            ss << " | " << std::setw(2) << std::setfill(' ') << std::dec << (int(ri.occurrences)) << " |\n";
        }

    return ss.str() + s;
}

static void go() {
    
    /*for (int material = 73; material >= 0; material--)
    {
        std::cout << material << ": " << (material*material*material/4096) << std::endl;
    }

    int total_material;

    Bitboard pawns  = bb(W_PAWN)   | bb(B_PAWN);
    Bitboard minors = bb(W_KNIGHT) | bb(B_KNIGHT) | bb(W_BISHOP) | bb(B_BISHOP);
    Bitboard rooks  = bb(W_ROOK)   | bb(B_ROOK);
    Bitboard queens = bb(W_QUEEN)  | bb(B_QUEEN);

    int num_pawns  = popcount(pawns);
    int num_minors = popcount(minors);
    int num_rooks  = popcount(rooks);
    int num_queens = popcount(queens);

    total_material = num_pawns + 3*num_minors + 5*num_rooks + 9*num_queens;

    std::cout << "total_material: " << total_material << std::endl;
    std::cout << static_eval<WHITE>() << std::endl;
    std::cout << static_eval<BLACK>() << std::endl;*/

    uint32_t u1[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    uint32_t u2[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    uint32_t u3[8];

    __m256i *a = (__m256i*)u1;
    __m256i *b = (__m256i*)u2;

    __m256i c = _mm256_add_epi32(*a, *b);

    _mm256_storeu_si256((__m256i*)(u3), c);

    for (int i = 0; i < 8; i++)
        std::cout << u3[i] << std::endl;
}

void Debug::go() {::go();}

Move *get_moves(Move *list)
{
#define populate(c) \
    { MoveList<c> moves; for (Move m : moves) *list++ = m; return list; }

    if (Position::white_to_move()) populate(WHITE) else populate(BLACK);
#undef populate
}
