
#ifndef TRANSPOSITIONTABLE_H
#define TRANSPOSITIONTABLE_H

#include "types.h"

#include <cstdint>

constexpr int TT_SIZE = 1 << 24;
constexpr int RT_SIZE = 1 << 15;

namespace RepetitionTable {
  bool has_repeated();
  void increment();
  void decrement();
  void clear();
}

struct RepInfo {
  uint64_t key;
  uint8_t occurrences;
};

using HashFlag = uint8_t;

enum HashFlags {
  EXACT,
  UPPER_BOUND,
  LOWER_BOUND
};

constexpr int FAIL = 0x7fffffff;

struct Entry {
  uint64_t key;
  uint8_t depth;
  HashFlag flag;
  int eval;
  uint16_t bestmove;
};

namespace TranspositionTable {

  int lookup(int depth, int alpha, int beta, int ply_from_root);
  void record(uint8_t depth, HashFlag flag, int eval, Move bestmove, int ply_from_root);
  Move lookup_move();
  void clear();

}

#endif
