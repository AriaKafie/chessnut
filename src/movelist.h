
#ifndef MOVELIST_H
#define MOVELIST_H

#include "types.h"

const int MAX_MOVES    = 128;
const int MAX_CAPTURES = 32;

template<Color Us>
class MoveList {

public:
    MoveList();

    Move  *begin()                 { return moves; }
    Move  *end()                   { return last; }
    size_t size()            const { return last - moves; }
    bool   in_check()        const { return ~checkmask; }
    Move   operator[](int i) const { return moves[i]; }

    void   sort(Move best_move, int ply_from_root);
    
    Move moves[MAX_MOVES], *last = moves;

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

    Move  *begin()               { return moves; }
    Move  *end()                 { return last; }
    size_t size()          const { return last - moves; }
    Move operator[](int i) const { return moves[i]; }

    void   sort();

private:
    void   insertion_sort();

    Move     moves[MAX_CAPTURES], *last = moves;
    Bitboard seen_by_enemy;

};

#endif

