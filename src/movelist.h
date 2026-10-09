
#ifndef MOVELIST_H
#define MOVELIST_H

#include "search.h"
#include "types.h"

typedef int Stage;
enum Stages {
    TT, GENERATE, GENERATED
};

const int MAX_MOVES    = 128;
const int MAX_CAPTURES = 32;

template<Color Us, Color Them = !Us>
class MoveList {

public:
    MoveList() {
        generate();
    }

    MoveList(Move tt, SearchInfo *si) : ttmove(tt), search_info(si) {
        if (ttmove) {
            *last++ = ttmove;
            stage = TT;
        } else {
            generate();
            *last = NULLMOVE;
            sort();
            stage = GENERATED;
        }
    }

    Move next_move() {
        
        switch (stage) {
            
        case TT:
            stage++;
            return *cur++;

        case GENERATE:
            stage++;
            last = moves;
            generate();
            *last = NULLMOVE;
            sort();
            return *cur++;

        case GENERATED:
        default:
            return *cur++;
        }
    }

    EMove* begin()      { return moves; }
    EMove* end()        { return last; }
    size_t size() const { return last - moves; }

private:
    void generate();
    void sort();
    void quicksort(int low, int high);
    int  partition(int low, int high);
    
    EMove       moves[MAX_MOVES];
    EMove*      last = moves;
    EMove*      cur = moves;
    SearchInfo* search_info;
    Bitboard    seen_by_enemy;
    Stage       stage;
    Move        ttmove;
};

template<Color Us, Color Them = !Us>
class CaptureList {

public:
    CaptureList();

    EMove* begin()      { return moves; }
    EMove* end()        { return last; }
    size_t size() const { return last - moves; }
    void   sort();

private:
    void   insertion_sort();

    EMove    moves[MAX_CAPTURES], *last = moves;
    Bitboard seen_by_enemy;

};

#endif
