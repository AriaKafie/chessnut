
#ifndef SEARCH_H
#define SEARCH_H

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include "moveordering.h"
#include "types.h"

inline constexpr int root_reductions[MAX_PLY] =
{
    0,1,1,1,1,2,2,2,2,2,
    2,2,2,2,2,3,3,3,3,3,
    3,3,3,3,3,3,3,3,3,3,
    3,3,3,3,3,3,3,3,3,3,
    4,4,4,4,4,4,4,4,4,4,
    5,5,5,5,5,5,5,5,5,5,
    6,6,6,6,6,6,6,6,6,6,
    7,7,7,7,7,7,7,7,7,7,
    8,8,8,8,8,8,8,8,8,8,
};

constexpr int matescore = 100000;

namespace Search
{
    void init();
    void go(uint64_t thinktime = 0);
    void count_nodes(int depth);

    inline void clear() { for (int ply = 0; ply < MAX_PLY; ply++) killers[ply].moveA = killers[ply].moveB = NO_MOVE; }
}

inline volatile bool search_cancelled;

inline void handle_search_stop(uint64_t thinktime)
{
    if (thinktime)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(thinktime));
        search_cancelled = true;
        return;
    }

    std::string in;
    do
        std::getline(std::cin, in);
    while (in != "stop");

    search_cancelled = true;
}

#endif
