
#ifndef TRANSPOSITIONTABLE_H
#define TRANSPOSITIONTABLE_H

#include "types.h"

#include <cstdint>

enum BoundType : uint8_t { EXACT, UPPER_BOUND, LOWER_BOUND };

/*
*  +---------------+-----+
*  | log2(TT_SIZE) | MiB |
*  +---------------+-----+
*  |            24 | 256 |
*  +---------------+-----+
*  |            23 | 128 |
*  +---------------+-----+
*  |            22 |  64 |
*  +---------------+-----+
*/

const int TT_SIZE = 1 << 23;
const int RT_SIZE = 1 << 10; // 16 KB

typedef struct {
    uint64_t  key;
    int       eval;
    uint16_t  best_move;
    uint8_t   depth;
    BoundType flag;
} TTEntry;

typedef struct {
    uint64_t key;
    uint8_t  occurrences;
} RTEntry;

namespace TranspositionTable
{
    int lookup(int depth, int alpha, int beta, int ply_from_root);
    void record(uint8_t depth, BoundType flag, int eval, Move best_move, int ply_from_root);
    Move lookup_move();
    void clear();
}

namespace RepetitionTable
{
    bool draw();
    void push();
    void pop();
    void clear();
}

#endif
