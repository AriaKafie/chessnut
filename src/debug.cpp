
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

static void go() {

    std::cout << "aa = " << (aa ? "true" : "false") << std::endl;
    std::cout << rep_table_to_string() << std::endl;
    return;

    int total_material;

    Bitboard pawns  = bb(W_PAWN)   | bb(B_PAWN);
    Bitboard minors = bb(W_KNIGHT) | bb(B_KNIGHT) | bb(W_BISHOP) | bb(B_BISHOP);
    Bitboard rooks  = bb(W_ROOK)   | bb(B_ROOK);
    Bitboard queens = bb(W_QUEEN)  | bb(B_QUEEN);

    int num_pawns  = popcount(pawns);
    int num_minors = popcount(minors);
    int num_rooks  = popcount(rooks);
    int num_queens = popcount(queens);

    total_material = num_pawns + 3*num_minors + 5*(num_rooks-1) + 9*num_queens;

    auto complexity = [](int i) -> int {
        return i*i*i/2048;
    };

    for (int mat = 0; mat <= total_material; mat++)
    {
        std::cout << "material=" << mat << ": " << complexity(mat) << std::endl;
    }

    

    /*uint32_t u1[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    uint32_t u2[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    uint32_t u3[8];

    __m256i *a = (__m256i*)u1;
    __m256i *b = (__m256i*)u2;

    __m256i c = _mm256_add_epi32(*a, *b);

    _mm256_storeu_si256((__m256i*)(u3), c);

    for (int i = 0; i < 8; i++)
        std::cout << u3[i] << std::endl;*/


    //int8_t i[32];
    //memset(i, -1, sizeof(i));
    //__m256i v = *(__m256i*)i;
    //__m256i zero = _mm256_setzero_si256();
    //__m256i sum8 = _mm256_sad_epu8(v, zero);  // 4x 64-bit partial sums
    //// Now horizontally add the four 64-bit elements:
    //__m128i hi = _mm256_extracti128_si256(sum8, 1);
    //__m128i lo = _mm256_castsi256_si128(sum8);
    //__m128i total = _mm_add_epi64(lo, hi);
    //uint64_t sum = _mm_cvtsi128_si64(total) + (uint64_t)_mm_extract_epi64(total, 1);
    //std::cout << (int)sum << std::endl;
    
}

void Debug::go() {::go();}

Move *get_moves(Move *list)
{
#define populate(c) \
    { MoveList<c> moves; for (Move m : moves) *list++ = m; return list; }

    if (Position::white_to_move()) populate(WHITE) else populate(BLACK);
#undef populate
}
