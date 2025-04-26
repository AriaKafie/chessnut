
#ifndef SEARCH_H
#define SEARCH_H

#include <algorithm>
#include <chrono>
#include <cassert>
#include <iostream>
#include <string>
#include <thread>
#include <array>

#include "types.h"

enum NodeType { ROOT, PV, NONPV };

/*template<typename T, int D>
class StatsEntry {

    T entry;

   public:
    StatsEntry& operator=(T v) {
        entry = v;
        return *this;
    }
    operator T() const { return entry; }

    void operator<<(int bonus) {
        int clamped = std::clamp(bonus, -D, D);
        entry += clamped - entry * std::abs(clamped) / D;

        assert(std::abs(entry) <= D);
    }
};*/

//typedef std::array<std::array<StatsEntry<int16_t, 30'000>, SQUARE_NB>, PIECE_NB> PieceToHistory;

typedef struct {
    //PieceToHistory *continuation_history;
    int  static_ev;
    int  ply;
    Move killers[2];
} SearchInfo;

inline uint64_t unix_ms() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

namespace Search {
    void clear();
    void noverbose();
    void init();
    void go(uint64_t thinktime = 0);
    void count_nodes(int depth);
}

/*struct ConthistBonus {
    int index;
    int weight;
};*/

#endif
