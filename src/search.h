
#ifndef SEARCH_H
#define SEARCH_H

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include "moveordering.h"
#include "types.h"

enum NodeType { ROOT, PV, NONPV };

typedef struct {
    int static_ev;
    int ply;
} SearchInfo;

struct RootMove {

    bool operator==(Move m) {
        return (m & 0xffff) == (move & 0xffff);
    }

    bool operator<(const RootMove& m) const {
        return m.score != score ? m.score < score : m.previous_score < previous_score;
    }

    Move move;
    int  score;
    int  previous_score;
    int  average_score;
    int  mean_squared_score;
};

inline uint64_t unix_ms() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

namespace Search
{
    void noverbose();

    void init();
    void go(uint64_t thinktime = 0);
    void count_nodes(int depth);

    inline void clear() {
        for (int ply = 0; ply < MAX_PLIES; ply++)
            killers[ply].moveA = killers[ply].moveB = NO_MOVE;
    }
}

#endif
