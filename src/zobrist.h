
#ifndef ZOBRIST_H
#define ZOBRIST_H

#include "types.h"

#include <cstdint>

namespace Zobrist {

  inline uint64_t key;
  inline uint64_t hash[B_KING + 1][SQUARE_NB];
  inline constexpr uint64_t BlackToMove = 17200288208102703589ull;
  void init();
  void set();
  void push();
  void pop();

}

#endif
