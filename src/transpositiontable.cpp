
#include "transpositiontable.h"
#include "position.h"

constexpr int TT_SIZE = 1 << 24;
constexpr int RT_SIZE = 1 << 20;

Entry transposition_table[TT_SIZE];
RepInfo repetition_table[RT_SIZE];

bool RepetitionTable::has_repeated() {
  const RepInfo& ri = repetition_table[Position::key() & (RT_SIZE - 1)];
  return ri.key == Position::key() && ri.occurrences >= 3;
}

void RepetitionTable::increment() {
  RepInfo& ri = repetition_table[Position::key() & (RT_SIZE - 1)];
  if (ri.key == Position::key())
    ri.occurrences++;
  else if (ri.key == 0 || ri.occurrences == 0) {
    ri.key = Position::key();
    ri.occurrences = 1;
  }
}

void RepetitionTable::decrement() {
  RepInfo& ri = repetition_table[Position::key() & (RT_SIZE - 1)];
  if (ri.key == Position::key())
    ri.occurrences--;
}

void RepetitionTable::clear() {
  memset(repetition_table, 0, RT_SIZE * sizeof(RepInfo));
}

int TranspositionTable::lookup(int depth, int alpha, int beta, int ply_from_root)
{
  if (const RepInfo& ri = repetition_table[Position::key() & (RT_SIZE - 1)]; ri.key == Position::key() && ri.occurrences > 1)
    return FAIL;

  const Entry& entry = transposition_table[Position::key() & (TT_SIZE - 1)];

  int eval = entry.eval;

  eval -= ply_from_root * (eval >  90000);
  eval += ply_from_root * (eval < -90000);

  if (entry.key == Position::key() && entry.depth >= depth) {
    if (entry.flag == EXACT)
      return eval;
    if (entry.flag == UPPER_BOUND && eval <= alpha)
      return alpha;
    if (entry.flag == LOWER_BOUND && eval >= beta)
      return beta;
  }

  return FAIL;
}

void TranspositionTable::record(uint8_t depth, HashFlag flag, int eval, Move bestmove, int ply_from_root) {
  eval += ply_from_root * (eval >  90000);
  eval -= ply_from_root * (eval < -90000);
  transposition_table[Position::key() & (TT_SIZE - 1)].set(Position::key(), depth, flag, eval, bestmove);
}

Move TranspositionTable::lookup_move() {
  const Entry& e = transposition_table[Position::key() & (TT_SIZE - 1)];
  return e.key == Position::key() ? e.bestmove : NULLMOVE;
}

void TranspositionTable::clear() {
  memset(transposition_table, 0, TT_SIZE * sizeof(Entry));
}
