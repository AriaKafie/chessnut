
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

/*#include <cmath>
#include <thread>
#include <time.h>

void search(int seed)
{
    Bitboard magic;

    bool visited[64], failed;

    std::mt19937_64 rng(seed);
    
    time_t start = time(NULL);

    do
    {
        memset(visited, false, 64);

        magic = rng();

        for (int i = 0; i < 64; i++)
        {
            int hash = (1ull << i) * magic >> (64 - 6);

            if (failed = visited[hash])
                break;

            else visited[hash] = true;
        }

    } while (failed);

    int seconds = time(NULL) - start;

    printf("seed %d: 0x%llx\nTime taken: %d minutes, %d seconds\n", seed, magic, seconds / 60, seconds % 60);
}*/

void Debug::go()
{
    /*printf("inline const int bitscan[64] {");

    int bitscan[64];
    for (int i = 0; i < 64; i++)
        bitscan[(1ull << i) * 0x756e2f651a4fcc2ull >> (64 - 6)] = lsb(1ull << i);

    for (int i = 0; i < 64; i++)
    {
        if (i % 8 == 0)
            printf("\n    ");

        printf("%d, ", bitscan[i]);
    }

    printf("\n};\n");*/

    std::cout << (RT_SIZE * sizeof(RTEntry) / 1024) << " KB" << std::endl
              << rep_table_to_string()                       << std::endl;

    /*int num_threads;
    std::cin >> num_threads;

    std::vector<std::thread> threads;

    for (int seed = 0; seed < num_threads; seed++)
        threads.push_back(std::thread(search, seed));

    for (std::thread& t : threads)
        t.join();*/
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
