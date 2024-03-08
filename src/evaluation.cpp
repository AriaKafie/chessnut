
#include "evaluation.h"
#include "util.h"
#include "board.h"
#include "gamestate.h"

#define bb(x) Board::bitboards[x]

ForceInline int midgame();
int endgame();
ForceInline int pawn_advancement();
ForceInline int king_safety();
int mopup();
ForceInline int material_count();
ForceInline int piece_placement();
ForceInline int doubled_pawns();

int static_eval() {
  return GameState::endgame ? endgame() : midgame();
}

ForceInline int midgame() {
  return material_count() + king_safety() + pawn_advancement() + piece_placement();
}

ForceInline int piece_placement() {
  int score = 0;
  for (PieceType pt : {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING}) {
    for (Bitboard b = bb(pt); b; pop_lsb(b)) {
      score += square_score[pt][lsb(b)^63];
    }
    for (Bitboard b = bb(pt + B_PAWN); b; pop_lsb(b)) {
      score -= square_score[pt][lsb(b)];
    }
  }
  return score;
}

ForceInline int pawn_advancement() {
  return 4 * (popcount(bb(W_PAWN) & RANK_567) 
              -  popcount(bb(B_PAWN) & RANK_234));
}

ForceInline int king_safety() {
  Square wksq = lsb(bb(W_KING));
  Square bksq = lsb(bb(B_KING));
  return KingSafety<WHITE>(wksq, bb(W_PAWN))
    - KingSafety<BLACK>(bksq, bb(B_PAWN));
}

ForceInline int doubled_pawns() {
  int score = 0;
  for (Bitboard file : {FILE_A, FILE_B, FILE_C, FILE_D,
                        FILE_E, FILE_F, FILE_G, FILE_H}) {
    score -= std::max(0, popcount(file & bb(W_PAWN)) - 1);
    score += std::max(0, popcount(file & bb(B_PAWN)) - 1);
  }
  return score * 4;
}

int endgame() {

  if (GameState::mopup) return mopup();
                
  int score = material_count();

  score += end_king_squares[lsb(Board::bitboards[W_KING])];
  score -= end_king_squares[lsb(Board::bitboards[B_KING])];
  score += 10 * popcount(bb(W_PAWN) & RANK_4);
  score += 20 * popcount(bb(W_PAWN) & RANK_5);
  score += 50 * popcount(bb(W_PAWN) & RANK_6);
  score += 90 * popcount(bb(W_PAWN) & RANK_7);
  score -= 10 * popcount(bb(B_PAWN) & RANK_5);
  score -= 20 * popcount(bb(B_PAWN) & RANK_4);
  score -= 50 * popcount(bb(B_PAWN) & RANK_3);
  score -= 90 * popcount(bb(B_PAWN) & RANK_2);

  return score;

}

int mopup() {

  int score = 0;
  if (GameState::white_computer) {
    score += CenterDistance(lsb(Board::bitboards[B_KING])) * 10;
    score += (14 - Distance(lsb(Board::bitboards[W_KING]),lsb(Board::bitboards[B_KING]))) * 4;
    return score + material_count();
  }
  score -= CenterDistance(lsb(Board::bitboards[W_KING])) * 10;
  score -= (14 - Distance(lsb(Board::bitboards[W_KING]),lsb(Board::bitboards[B_KING]))) * 4;
  return score + material_count();

}

ForceInline int material_count() {

  int score = 100 * (popcount(bb(W_PAWN))                  - popcount(bb(B_PAWN)));
  score    += 300 * (popcount(bb(W_KNIGHT) | bb(W_BISHOP)) - popcount(bb(B_KNIGHT) | bb(B_BISHOP)));
  score    += 500 * (popcount(bb(W_ROOK))                  - popcount(bb(B_ROOK)));
  score    += 900 * (popcount(bb(W_QUEEN))                 - popcount(bb(B_QUEEN)));
  return score;

}

#undef bb
