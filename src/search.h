
#ifndef SEARCH_H
#define SEARCH_H

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include "moveordering.h"
#include "types.h"

typedef struct {
    int  static_ev;
} Ply;

const int matescore = 100000;

inline uint64_t unix_ms() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

namespace Search
{
    typedef struct {
        Move          root_move;
        int           nodes;
        int           root_delta;
        volatile bool search_cancelled;
        bool          verbose;
    } Status;

    inline Status status;

    void init();
    void go(uint64_t thinktime = 0);
    void count_nodes(int depth);

    inline void clear() {
        for (int ply = 0; ply < MAX_DEPTH; ply++)
            killers[ply].moveA = killers[ply].moveB = NO_MOVE;
    }
}

inline void handle_search_stop(uint64_t thinktime)
{
    if (thinktime)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(thinktime));
        Search::status.search_cancelled = true;
        return;
    }

    std::string in;
    do
        std::getline(std::cin, in);
    while (in != "stop");

    Search::status.search_cancelled = true;
}

#endif
