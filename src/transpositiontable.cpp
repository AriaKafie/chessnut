
#include "transpositiontable.h"
#include "position.h"

TTEntry transposition_table[TT_SIZE];
RTEntry repetition_table[RT_SIZE];

bool RepetitionTable::draw()
{
    RTEntry *r = &repetition_table[Position::key() & (RT_SIZE - 1)];
    return r->key == Position::key() && r->occurrences >= 3;
}

void RepetitionTable::push()
{
    RTEntry *r = &repetition_table[Position::key() & (RT_SIZE - 1)];

    if (r->key == Position::key())
    {
        r->occurrences++;
    }
    else if (r->occurrences == 0)
    {
        r->key = Position::key();
        r->occurrences = 1;
    }
}

void RepetitionTable::pop()
{
    RTEntry *r = &repetition_table[Position::key() & (RT_SIZE - 1)];

    if (r->key == Position::key())
        r->occurrences--;
}

void RepetitionTable::clear() {
    memset(repetition_table, 0, RT_SIZE * sizeof(RTEntry));
}

int TranspositionTable::lookup(int depth, int alpha, int beta, int ply_from_root)
{
    if (RTEntry *r = &repetition_table[Position::key() & (RT_SIZE - 1)]; r->key == Position::key() && r->occurrences > 1)
        return NO_EVAL;

    TTEntry *entry = &transposition_table[Position::key() & (TT_SIZE - 1)];

    int eval = entry->eval;

    eval -= ply_from_root * is_win(eval);
    eval += ply_from_root * is_loss(eval);

    if (entry->key == Position::key() && entry->depth >= depth)
    {
        if (entry->flag == EXACT)
            return eval;

        if (entry->flag == UPPER_BOUND && eval <= alpha)
            return alpha;

        if (entry->flag == LOWER_BOUND && eval >= beta)
            return beta;
    }

    return NO_EVAL;
}

void TranspositionTable::record(uint8_t depth, BoundType flag, int eval, Move best_move, int ply_from_root)
{
    eval += ply_from_root * is_win(eval);
    eval -= ply_from_root * is_loss(eval);

    TTEntry *e = &transposition_table[Position::key() & (TT_SIZE - 1)];

    e->key       = Position::key();
    e->depth     = depth;
    e->flag      = flag;
    e->eval      = eval;
    e->best_move = best_move;
}

Move TranspositionTable::lookup_move()
{
    TTEntry *e = &transposition_table[Position::key() & (TT_SIZE - 1)];
    return e->key == Position::key() ? e->best_move : NO_MOVE;
}

void TranspositionTable::clear() {
    memset(transposition_table, 0, TT_SIZE * sizeof(TTEntry));
}
