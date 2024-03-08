
#include "transpositiontable.h"
#include "zobrist.h"
#include "gamestate.h"
#include <iostream>

Entry entries[tablesize];

namespace TranspositionTable {

  int lookup(int depth, int alpha, int beta, int ply_from_root) {

    Entry* entry = &entries[Zobrist::key & modulo];
    int eval = entry->eval;
    eval -= ply_from_root * (eval > 90000);
    eval += ply_from_root * (eval < -90000);

    if (entry->key == Zobrist::key) {
      if (entry->depth >= depth) {
        if (entry->flag == EXACT)
          return eval;
        if ((entry->flag == UPPER_BOUND) &&
            (alpha >= eval))
          return alpha;
        if ((entry->flag == LOWER_BOUND) &&
            (beta <= eval))
          return beta;
      }
    }
    return FAIL;
  }

  void disable() {
    disabled = true;
  }

  void enable() {
    disabled = false;
  }

  void record(uint8_t depth, HashFlag flag, int eval, uint16_t best_move, int ply_from_root) {
    int index = Zobrist::key & modulo;
    eval += ply_from_root * (eval > 90000);
    eval -= ply_from_root * (eval < -90000);
    entries[index].set(Zobrist::key, depth, flag, eval, best_move);
  }

  int lookup_move() {
    return entries[Zobrist::key & modulo].best_move;
  }

  void clear() {
    for (int i = 0; i < tablesize; i++) {
      entries[i].set(0,0,0,0,0);
    }
  }

} // namespace TranspositionTable

