
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

#include "movegen.h"
#include "moveordering.h"
#include "position.h"
#include "transpositiontable.h"
#include "uci.h"

template<Color STM>
std::string PV()
{
    if (RepetitionTable::draw())
        return "";

    std::string line;
    Move best = TranspositionTable::lookup_move();

    if (best == NO_MOVE)
        return "";

    else
    {
        do_move<STM>(best);
        line = move_to_uci(best) + " " + PV<!STM>();
        undo_move<STM>(best);
    }

    return line;
}

std::string Debug::pv()
{
    return Position::white_to_move() ? PV<WHITE>()
                                     : PV<BLACK>();
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

Bitboard passer_mask(Bitboard pawns, Color c)
{
    Bitboard scope = 0ull;

    for (;pawns; clear_lsb(pawns))
    {
        Square psq = lsb(pawns);

        scope |= (file_bb(psq) | file_bb(psq + EAST) & ~FILE_ABB | file_bb(psq + WEST) & ~FILE_HBB) & mask(psq, c == WHITE ? SOUTH : NORTH);
    }

    return ~scope;
}

int colex(int i)
{
    int t = i | (i-1);
    return (t+1) | ((~t & -(~t))-1)>>(lsb(i)+1);
}

Bitboard image[4][1 << 10];

void Debug::go()
{
    Color c = BLACK;

    Bitboard relevancy = relative_rank_bb(c, RANK_7)
                       | relative_rank_bb(c, RANK_6)
                       | relative_rank_bb(c, RANK_5)
                       | relative_rank_bb(c, RANK_4)
                       | relative_rank_bb(c, RANK_3);

    Bitboard masks_[4] = {relevancy & (relative_file_bb(c, FILE_G) | relative_file_bb(c, FILE_H)),
                          relevancy & (relative_file_bb(c, FILE_E) | relative_file_bb(c, FILE_F)),
                          relevancy & (relative_file_bb(c, FILE_C) | relative_file_bb(c, FILE_D)),
                          relevancy & (relative_file_bb(c, FILE_A) | relative_file_bb(c, FILE_B)) };

    for (int i = 0; i < 4; i++)
        printf("0x%llxull\n", masks_[i]);return;

    Bitboard pdep(Bitboard b, int i);

    Bitboard m1 = masks_[0];
    Bitboard m2 = masks_[1];
    Bitboard m3 = masks_[2];
    Bitboard m4 = masks_[3];

    Bitboard masks[4] = {m1,m2,m3,m4};

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 1 << popcount(masks[i]); j++)
        {
            Bitboard pawns = pdep(masks[i], j);

            image[i][j] = passer_mask(pawns, WHITE);
        }
    }

    for (;;)
    {
        static std::mt19937_64 rng(0);

        Bitboard pawns = rng() & rng() & rng() & ~(RANK_1BB | relative_rank_bb(c, RANK_2) | RANK_8BB);

        Bitboard passers = Passers[c][0][pext(pawns, m1)]
                         & Passers[c][1][pext(pawns, m2)]
                         & Passers[c][2][pext(pawns, m3)]
                         & Passers[c][3][pext(pawns, m4)];
        
        std::cout << to_string(pawns) << to_string(passers) << std::endl;

        if (passers != passer_mask(pawns, c))
        {
            std::cout << "fail\n";
            std::exit(0);
        }
    }

    return;
    std::cout << (RT_SIZE * sizeof(RTEntry) / 1024) << " KB" << std::endl
              << rep_table_to_string()                       << std::endl;
}

Move *get_moves(Move *list)
{
    if (Position::white_to_move())
    {
        MoveList<WHITE> m;
        memcpy(list, m.moves, sizeof(Move) * m.size());
        return list + m.size();
    }
    else
    {
        MoveList<BLACK> m;
        memcpy(list, m.moves, sizeof(Move) * m.size());
        return list + m.size();
    }
}
