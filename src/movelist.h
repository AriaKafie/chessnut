
#ifndef MOVELIST_H
#define MOVELIST_H

#include "types.h"

constexpr int MAX_MOVES    = 128;
constexpr int MAX_CAPTURES = 16;

template<Color Us>
class MoveList {

public:
    MoveList();

    Move*  begin()                 { return moves; }
    Move*  end()                   { return last; }
    size_t size()            const { return last - moves; }
    bool   in_check()        const { return ~checkmask; }
    Move   operator[](int i) const { return moves[i]; }

    void   sort(Move best_move, int ply);

private:
    void quicksort(int low, int high);

    Move     moves[MAX_MOVES];
    Move    *last;
    Bitboard checkmask;
    Bitboard seen_by_enemy;
    int      partition(int low, int high);

};

template<Color Us>
class CaptureList {

public:
    CaptureList();

    Move*  begin()               { return moves; }
    Move*  end()                 { return last; }
    size_t size()          const { return last - moves; }
    Move operator[](int i) const { return moves[i]; }

    void     sort();

private:
    void insertion_sort();

    Move     moves[MAX_CAPTURES];
    Move    *last;
    Bitboard checkmask;
    Bitboard seen_by_enemy;

};

#endif

