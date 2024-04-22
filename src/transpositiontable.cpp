
#include "transpositiontable.h"
#include "position.h"

#include <iostream>

constexpr int TTSize = 1 << 24;
constexpr int RepSize = 1 << 12;

Entry entries[TTSize];
RepInfo rep_table[RepSize];

bool RepetitionTable::has_repeated() {
  RepInfo& ri = rep_table[Position::key() & (RepSize - 1)];
  if (ri.key == Position::key() && ri.occurrences >= 3)
    return true;
  return false;
}

void RepetitionTable::increment() {
  RepInfo& ri = rep_table[Position::key() & (RepSize - 1)];
  if (ri.key == Position::key())
    ri.occurrences++;
  else if (ri.key == 0 || ri.occurrences == 0) {
    ri.key = Position::key();
    ri.occurrences = 1;
  }
}

void RepetitionTable::decrement() {
  RepInfo& ri = rep_table[Position::key() & (RepSize - 1)];
  if (ri.key == Position::key())
    ri.occurrences--;
}

void RepetitionTable::clear() {
  memset(rep_table, 0, RepSize * sizeof(RepInfo));
}

int TranspositionTable::lookup(int depth, int alpha, int beta, int ply_from_root) {

  Entry* entry = &entries[Position::key() & (TTSize - 1)];
  int eval = entry->eval;
  eval -= ply_from_root * (eval > 90000);
  eval += ply_from_root * (eval < -90000);

  if (entry->key == Position::key()) {
    if (entry->depth >= depth) {
      if (entry->flag == EXACT)
        return eval;
      if (entry->flag == UPPER_BOUND && alpha >= eval)
        return alpha;
      if (entry->flag == LOWER_BOUND && beta <= eval)
        return beta;
    }
  }
  return FAIL;
}

void TranspositionTable::record(uint8_t depth, HashFlag flag, int eval, uint16_t best_move, int ply_from_root) {
  int index = Position::key() & (TTSize - 1);
  eval += ply_from_root * (eval > 90000);
  eval -= ply_from_root * (eval < -90000);
  entries[index].set(Position::key(), depth, flag, eval, best_move);
}

int TranspositionTable::lookup_move() {
  return entries[Position::key() & (TTSize - 1)].best_move;
}

void TranspositionTable::clear() {
  memset(entries, 0, TTSize * sizeof(Entry));
}
