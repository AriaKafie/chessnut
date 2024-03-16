
#ifndef MOVEORDERING_H
#define MOVEORDERING_H

#include "board.h"
#include "movelist.h"
#include "evaluation.h"

struct Killer {

  Move moveA;
  Move moveB;

  void add(Move m) {
    if (m != moveA) {
      moveB = moveA;
      moveA = m;
    }
  }

  bool contains(Move m) const {
    return (m == moveA) || (m == moveB);
  }

};

inline Killer killer_moves[256];

constexpr uint32_t MAX_SCORE             = 0xffff0000;
constexpr uint32_t WINNING_CAPTURE_BONUS = 8000;
constexpr uint32_t LOSING_CAPTURE_BONUS  = 2000;
constexpr uint32_t KILLER_BONUS          = 4000;
constexpr uint32_t SEEN_BY_PAWN_PENALTY  = -50;

template<Color Us>
void CaptureList<Us>::insertion_sort() {
  
  for (int i = 1; i < length(); i++) {
    Move key = moves[i];
    int j = i - 1;
    while (j >= 0 && score_of(moves[j]) < score_of(key)) {
      moves[j + 1] = moves[j];
      j--;
    }
    moves[j + 1] = key;
  }

}

template<Color Us>
void CaptureList<Us>::sort() {

  constexpr Color Them      = !Us;
  constexpr Piece EnemyPawn = make_piece(Them, PAWN);
  
  Bitboard seen_by_pawn = PawnAttacks<Them>(bb(EnemyPawn));
            
  for (Move& m : *this) {
    
    uint32_t score = 1000;
    
    Square    to       = to_sq(m);
    PieceType captured = piece_type_on(to);
    
    if (square_bb(to) & seen_by_pawn)
      score -= 500;
    score += piece_weight(captured) << 1;

    m += score << 16;
      
  }
  
  insertion_sort();
  
}

template<Color Us>
int MoveList<Us>::partition(int low, int high) {
  uint32_t pivot = score_of(moves[high]);
  int i = low - 1;
  for (int j = low; j < high; j++) {
    if (score_of(moves[j]) >= pivot) {
      i++;
      std::swap(moves[i], moves[j]);
    }
  }
  std::swap(moves[i + 1], moves[high]);
  return i + 1;
}

template<Color Us>
void MoveList<Us>::quicksort(int low, int high) {
  if (low < high) {
    int pivot_index = partition(low, high);
    quicksort(low, pivot_index - 1);
    quicksort(pivot_index + 1, high);
  }
}

template<Color Us>
void MoveList<Us>::sort(Move pv, int ply) {

  constexpr Color Them      = !Us;
  constexpr Piece EnemyPawn = make_piece(Them, PAWN);
  
  Bitboard seen_by_pawn = PawnAttacks<Them>(bb(EnemyPawn));
              
  for (Move& m : *this) {
    
    if (m == (pv & 0xffff)) {
      m += MAX_SCORE;
      continue;
    }
    
    uint32_t score = 1000;
    
    Square    from     = from_sq(m);
    Square    to       = to_sq(m);
    PieceType from_pt  = piece_type_on(from);
    PieceType captured = piece_type_on(to);
    
    if (captured) {
      int material_delta = piece_weight(captured) - piece_weight(from_pt);
      if (square_bb(to) & seen_by_enemy)
        score += (material_delta >= 0 ? WINNING_CAPTURE_BONUS : LOSING_CAPTURE_BONUS) + material_delta;
      else
        score += WINNING_CAPTURE_BONUS + material_delta;
    }
    else {
      if (killer_moves[ply].contains(m))
        score += KILLER_BONUS;
      score += (square_score<Us>(from_pt, to) - square_score<Us>(from_pt, from)) / 2;
    }
    
    if (square_bb(to) & seen_by_pawn)
      score += SEEN_BY_PAWN_PENALTY;

    m += score << 16;
      
  }
  
  quicksort(0, length()-1);
  
}

#endif
