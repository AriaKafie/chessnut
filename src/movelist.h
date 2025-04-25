
#ifndef MOVELIST_H
#define MOVELIST_H

#include "search.h"
#include "types.h"

const int MAX_MOVES    = 128;
const int MAX_CAPTURES = 32;

template<Color Us>
class MoveList {

public:
    MoveList();

    LMove* begin()          { return moves; }
    LMove* end()            { return last; }
    size_t size()     const { return last - moves; }
    bool   in_check() const { return ~checkmask; }
    void   sort(Move ttmove, SearchInfo *si);
    
    LMove moves[MAX_MOVES], *last = moves;

private:
    Bitboard checkmask;
    Bitboard seen_by_enemy;

    void quicksort(int low, int high);
    int  partition(int low, int high);
};

template<Color Us>
class CaptureList {

public:
    CaptureList();

    LMove* begin()      { return moves; }
    LMove* end()        { return last; }
    size_t size() const { return last - moves; }
    void   sort();

private:
    void   insertion_sort();

    LMove    moves[MAX_CAPTURES], *last = moves;
    Bitboard seen_by_enemy;

};

#endif

