
#ifndef SEARCH_H
#define SEARCH_H

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include "moveordering.h"
#include "types.h"

constexpr int matescore = 100000;

struct Status {
    inline static Move          root_move;
    inline static int           nodes;
    inline static volatile bool search_cancelled;
};

namespace Search
{
    void init();
    void go(uint64_t thinktime = 0);
    void count_nodes(int depth);

    inline void clear() { for (int ply = 0; ply < MAX_PLY; ply++) killers[ply].moveA = killers[ply].moveB = NO_MOVE; }
}

inline void handle_search_stop(uint64_t thinktime)
{
    if (thinktime)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(thinktime));
        Status::search_cancelled = true;
        return;
    }

    std::string in;
    do
        std::getline(std::cin, in);
    while (in != "stop");

    Status::search_cancelled = true;
}

#endif
