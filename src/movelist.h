
#ifndef MOVELIST_H
#define MOVELIST_H

#include "types.h"

constexpr int MAX_MOVES  = 128;
constexpr int MAX_CMOVES = 16;

template<Color Us>
class MoveList {

public:
  MoveList(bool ep_enabled = true);
  Move*    begin()         { return moves; }
  Move*    end()           { return last; }
  size_t   length()  const { return last - moves; }
  bool     incheck() const { return ~checkmask; }
  void     sort(Move pv, int ply);
  Move operator[](int i) const {
    return moves[i];
  }

private:
  Move     moves[MAX_MOVES],
           *last;
  Bitboard checkmask;
  Bitboard seen_by_enemy;
  void     quicksort(int low, int high);
  int      partition(int low, int high);

};

template<Color Us>
class CaptureList {

public:
  CaptureList();
  Move*    begin()        { return moves; }
  Move*    end()          { return last; }
  size_t   length() const { return last - moves; }
  void     sort();
  Move operator[](int i) const {
    return moves[i];
  }

private:
  Move     moves[MAX_CMOVES],
           *last;
  Bitboard checkmask;
  Bitboard seen_by_enemy;
  void     insertion_sort();

};

#endif

