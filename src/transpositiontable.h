
#ifndef TRANSPOSITIONTABLE_H
#define TRANSPOSITIONTABLE_H

#include "types.h"

#include <cstdint>

//   entries    megabytes
// 
//   4194304    67
//   16777216   268

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
  Move bestmove;
  void set(uint64_t k, uint8_t d, HashFlag f, int e,  Move m) {
    key = k; depth = d; flag = f; eval = e; bestmove = m;
  }
};

namespace TranspositionTable {

  int lookup(int depth, int alpha, int beta, int ply_from_root);
  void record(uint8_t depth, HashFlag flag, int eval, Move bestmove, int ply_from_root);
  int lookup_move();
  void clear();

}

#endif
