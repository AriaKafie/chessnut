
#ifndef MOVEORDERING_H
#define MOVEORDERING_H

#include "board.h"
#include "movelist.h"
#include "evaluation.h"

#define bb(x) Board::bitboards[x]

struct Killer {

  int moveA;
  int moveB;

  void add(int move) {
    if (move != moveA) {
      moveB = moveA;
      moveA = move;
    }
  }

  bool contains(int move) const {
    return (move == moveA) || (move == moveB);
  }

};

inline Killer killer_moves[256];

constexpr int MAX_SCORE             = 0x7fff0000;
constexpr int WINNING_CAPTURE_BONUS = 8000;
constexpr int LOSING_CAPTURE_BONUS  = 2000;
constexpr int KILLER_BONUS          = 4000;
constexpr int SEEN_BY_PAWN_PENALTY  = -50;

template<Color Side>
void CaptureList<Side>::insertion_sort() {
  
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

template<Color Side>
void CaptureList<Side>::sort() {

  constexpr Color    Enemy        = flip<Side>();
  constexpr Piece    EnemyPawn    = piece<Enemy, PAWN>();
            Bitboard seen_by_pawn = PawnAttacks<Enemy>(bb(EnemyPawn));
            
  for (Move& m : *this) {
    
    int score = 1000;
    
    Square to = to_sq(m);
    PieceType to_pt = piece_type<Enemy>(Board::pieces[to]);
    
    if (square_bb(to) & seen_by_pawn)
      score -= 500;
    score += piece_weight[to_pt] << 1;

    m += score << 16;
      
  }
  
  insertion_sort();
  
}

template<Color Side>
int MoveList<Side>::partition(int low, int high) {
  int pivot = score_of(moves[high]);
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

template<Color Side>
void MoveList<Side>::quicksort(int low, int high) {
  if (low < high) {
    int pivot_index = partition(low, high);
    quicksort(low, pivot_index - 1);
    quicksort(pivot_index + 1, high);
  }
}

template<Color Side>
void MoveList<Side>::sort(Move pv, int ply) {

  constexpr Color    Enemy        = flip<Side>();
  constexpr Piece    EnemyPawn    = piece<Enemy, PAWN>();
            Bitboard seen_by_pawn = PawnAttacks<Enemy>(bb(EnemyPawn));
  
  constexpr int t = Side == WHITE ? 63 : 0;
            
  for (Move& m : *this) {
    
    if (m == (pv & 0xffff)) {
      m += MAX_SCORE;
      continue;
    }
    
    int score = 1000;
    
    Square    from = from_sq(m);
    Square    to   = to_sq(m);
    PieceType f_pt = piece_type<Side >(Board::pieces[from]);
    PieceType t_pt = piece_type<Enemy>(Board::pieces[to  ]);
    
    if (Board::pieces[to] != NO_PIECE) {
      int material_delta = piece_weight[t_pt] - piece_weight[f_pt];
      if (square_bb(to) & seen_by_enemy)
        score += (material_delta >= 0 ? WINNING_CAPTURE_BONUS : LOSING_CAPTURE_BONUS) + material_delta;
      else
        score += WINNING_CAPTURE_BONUS + material_delta;
    }
    else {
      if (killer_moves[ply].contains(m))
        score += KILLER_BONUS;
      score += (square_score[f_pt][to^t] - square_score[f_pt][from^t]) / 2;
    }
    
    if (square_bb(to) & seen_by_pawn)
      score += SEEN_BY_PAWN_PENALTY;

    m += score << 16;
      
  }
  
  quicksort(0, length()-1);
  
}

#undef bb

#endif
