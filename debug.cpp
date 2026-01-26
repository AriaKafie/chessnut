
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

#include "evaluation.h"
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

        double elapsed = std::chrono::duration<double>(end - start).count();

        printf(R"(
Nodes searched: %llu

===========================
Total time (s) : %.3f
Nodes searched : %llu
Nodes/second   : %llu)""\n\n",
        result, elapsed, result, uint64_t(result / elapsed));
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

    std::ostringstream os;
    std::string s = "+------------------+------+----+\n";

    os << s << "| key              | loc  |  # |\n" << s;

    for (int i = 0; i < RT_SIZE; i++) {
        if (RTEntry& ri = repetition_table[i]; ri.occurrences) {
            os << "| " << std::setw(16) << std::setfill('0') << std::hex << std::uppercase << ri.key;
            os << " | " << std::setw(4) << std::setfill('0') << std::hex << std::uppercase << i;
            os << " | " << std::setw(2) << std::setfill(' ') << std::dec << (int(ri.occurrences)) << " |\n";
        }
    }

    return os.str() + s;
}

Bitboard negative_spans(Color c, Bitboard pawns)
{
    Bitboard spans = 0;

    for (;pawns; clear_lsb(pawns))
    {
        Square s = bsf(pawns);
        spans |= (file_bb(s) | file_bb(s + EAST) & ~FILE_ABB | file_bb(s + WEST) & ~FILE_HBB) & mask(s, relative_direction(c, SOUTH));
    }

    return ~spans;
}

static void go() {

    Bitboard pdep(Bitboard b, int i);

    Bitboard pawns[1024];
    Bitboard neg_spans[1024];

    Color c = WHITE;
    Bitboard relevancy = relative_rank(c, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7);
    Bitboard masks[4] = {
        relevancy & relative_file(c, FILE_G, FILE_H),
        relevancy & relative_file(c, FILE_E, FILE_F),
        relevancy & relative_file(c, FILE_C, FILE_D),
        relevancy & relative_file(c, FILE_A, FILE_B)
    };
    
    for (int i = 0; i < 1024; i++)
    {
        pawns[i]     = pdep(masks[0], i);           // i'th configuration of pawns, i : [0, 1024)
        neg_spans[i] = negative_spans(c, pawns[i]); // corresponding ~spans
    }

    bool visited[1024], failed;
    int shift = 64 - 9;

    std::mt19937_64 rng(0);

    Bitboard ref[1024], magic;
    int max_i = 0;
    do
    {
        failed = false;
        magic = rng() & rng();
        memset(visited, false, 1024);

        for (int i = 0; i < 1024; i++)
        {
            uint64_t key = pawns[i] * magic >> shift;

            if (visited[key] && ref[key] != neg_spans[i])
            {
                if (i > max_i)
                {
                    max_i = i;
                    std::cout << i << "/1024" << std::endl;
                }
                failed = true;
                break;
            }

            visited[key] = true;
            ref[key] = neg_spans[i];
        }

    } while (failed);

    std::cout << std::hex << magic << std::endl;
}

void Debug::go() {::go();}

Move *get_moves(Move *list)
{
#define populate(c) \
    { MoveList<c> moves; for (Move m : moves) *list++ = m; return list; }

    if (Position::white_to_move()) populate(WHITE) else populate(BLACK);
#undef populate
}
