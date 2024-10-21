
#include "transpositiontable.h"
#include "position.h"

Entry transposition_table[TT_SIZE];
RepInfo repetition_table[RT_SIZE];

bool RepetitionTable::draw() {

    const RepInfo& ri = repetition_table[Position::key() & (RT_SIZE - 1)];

    return ri.key == Position::key() && ri.occurrences >= 3;
}

void RepetitionTable::push() {

    RepInfo& ri = repetition_table[Position::key() & (RT_SIZE - 1)];

    if (ri.key == Position::key())
        ri.occurrences++;
    else if (ri.key == 0 || ri.occurrences == 0)
    {
        ri.key = Position::key();
        ri.occurrences = 1;
    }
}

void RepetitionTable::pop() {

    RepInfo& ri = repetition_table[Position::key() & (RT_SIZE - 1)];

    if (ri.key == Position::key())
        ri.occurrences--;
}

void RepetitionTable::clear() {
    memset(repetition_table, 0, RT_SIZE * sizeof(RepInfo));
}

int TranspositionTable::lookup(int depth, int alpha, int beta, int ply_from_root) {

    if (const RepInfo& ri = repetition_table[Position::key() & (RT_SIZE - 1)]; ri.key == Position::key() && ri.occurrences > 1)
        return NO_EVAL;

    const Entry& entry = transposition_table[Position::key() & (TT_SIZE - 1)];

    int eval = entry.eval;

    eval -= ply_from_root * (eval >  90000);
    eval += ply_from_root * (eval < -90000);

    if (entry.key == Position::key() && entry.depth >= depth)
    {
        if (entry.flag == EXACT)
            return eval;
        if (entry.flag == UPPER_BOUND && eval <= alpha)
            return alpha;
        if (entry.flag == LOWER_BOUND && eval >= beta)
            return beta;
    }

    return NO_EVAL;
}

void TranspositionTable::record(uint8_t depth, BoundType flag, int eval, Move best_move, int ply_from_root) {

    eval += ply_from_root * (eval >  90000);
    eval -= ply_from_root * (eval < -90000);

    Entry& e = transposition_table[Position::key() & (TT_SIZE - 1)];

    e.key       = Position::key();
    e.depth     = depth;
    e.flag      = flag;
    e.eval      = eval;
    e.best_move = best_move;
}

Move TranspositionTable::lookup_move() {

    const Entry& entry = transposition_table[Position::key() & (TT_SIZE - 1)];

    return entry.key == Position::key() ? entry.best_move : NO_MOVE;
}

void TranspositionTable::clear() {
    memset(transposition_table, 0, TT_SIZE * sizeof(Entry));
}
