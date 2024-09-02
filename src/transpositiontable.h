
#ifndef TRANSPOSITIONTABLE_H
#define TRANSPOSITIONTABLE_H

#include "types.h"

#include <cstdint>

using HashFlag = uint8_t;

constexpr int FAIL = 0x7fffffff;

constexpr int TT_SIZE = 1 << 24;
constexpr int RT_SIZE = 1 << 15;

enum HashFlags {
    EXACT,
    UPPER_BOUND,
    LOWER_BOUND
};

struct Entry {
    uint64_t key;
    uint8_t  depth;
    HashFlag flag;
    int      eval;
    uint16_t best_move;
};

struct RepInfo {
    uint64_t key;
    uint8_t  occurrences;
};

namespace TranspositionTable {
    int lookup(int depth, int alpha, int beta, int ply_from_root);
    void record(uint8_t depth, HashFlag flag, int eval, Move best_move, int ply_from_root);
    Move lookup_move();
    void clear();
}

namespace RepetitionTable {
    bool has_repeated();
    void increment();
    void decrement();
    void clear();
}

#endif
