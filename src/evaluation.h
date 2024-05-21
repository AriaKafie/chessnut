
#ifndef EVALUATION_H
#define EVALUATION_H

#include "position.h"
#include "types.h"

int static_eval();

constexpr int piece_weight_table[KING + 1] = { 0, 0, 100, 300, 300, 500, 900, 1500 };

inline int piece_weight(PieceType pt) { return piece_weight_table[pt]; }

constexpr int square_score_table[PIECE_TYPE_NB][SQUARE_NB] = 
{
// scored from black's pov (promotion = 0-7) with a maximizer perspective
  { // pawn
    50, 50, 50, 50, 50, 50, 50, 50,
    50, 50, 50, 50, 50, 50, 50, 50,
    10, 10, 20, 30, 30, 20, 10, 10,
    5,  5, 10, 25, 25, 10,  5,  5,
    0,  0,  0, 20, 20,  0,  0,  0,
    5, -5,-10,  0,  0,-10, -5,  5,
    5, 10, 10,-20,-20, 10, 10,  5,
    0,  0,  0,  0,  0,  0,  0,  0
  },
  { // knight
    -50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50,
  },
  { // bishop
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -20,-10,-10,-10,-10,-10,-10,-20,
  },
  { // rook
    0,  0,  0,  0,  0,  0,  0,  0,
    5, 10, 10, 10, 10, 10, 10,  5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  5,  5,  0,  0, -5
  },
  { // queen
    -20,-10,-10, -5, -5,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5,  5,  5,  5,  0,-10,
    -5,  0,  5,  5,  5,  5,  0, -5,
    0,  0,  5,  5,  5,  5,  0, -5,
    -10,  5,  5,  5,  5,  5,  0,-10,
    -10,  0,  5,  0,  0,  0,  0,-10,
    -20,-10,-10, -5, -5,-10,-10,-20
  },
  { // king
    -6,-8,-8,-10,-10,-8,-8,-6,
    -6,-8,-8,-10,-10,-8,-8,-6,
    -6,-8,-8,-10,-10,-8,-8,-6,
    -6,-8,-8,-10,-10,-8,-8,-6,
    -4,-6,-6,-8, -8, -6,-6,-4,
    -2,-4,-4,-4, -4, -4,-4,-2,
    -2,-2,-1,-1, -1, -1,-2,-2,
    0, 0, 0, 0,  0,  0, 0, 0,
  }
};

template<Color Perspective>
constexpr int square_score(PieceType pt, Square sq) {
  if constexpr (Perspective == WHITE)
    return square_score_table[pt - 2][sq ^ 63];
  else
    return square_score_table[pt - 2][sq];
}

constexpr int end_king_squares[] = {
  -10,-8,-6,-4,-4,-6,-8,-10,
  -6, -4,-2, 0, 0,-2,-4,-6,
  -6, -2, 4, 6, 6, 4,-2,-6,
  -6, -2, 6, 8, 8, 6,-2,-6,
  -6, -2, 6, 8, 8, 6,-2,-6,
  -6, -2, 4, 6, 6, 4,-2,-6,
  -6, -6, 0, 0, 0, 0,-6,-6,
  -10,-6,-6,-6,-6,-6,-6,-10,
};

template<Color Us>
int material_count()
{
  constexpr Color Them = !Us;

  constexpr Piece FPAWN   = make_piece(Us,   PAWN);
  constexpr Piece EPAWN   = make_piece(Them, PAWN);
  constexpr Piece FKNIGHT = make_piece(Us,   KNIGHT);
  constexpr Piece EKNIGHT = make_piece(Them, KNIGHT);
  constexpr Piece FBISHOP = make_piece(Us,   BISHOP);
  constexpr Piece EBISHOP = make_piece(Them, BISHOP);
  constexpr Piece FROOK   = make_piece(Us,   ROOK);
  constexpr Piece EROOK   = make_piece(Them, ROOK);
  constexpr Piece FQUEEN  = make_piece(Us,   QUEEN);
  constexpr Piece EQUEEN  = make_piece(Them, QUEEN);
  
  return
    100 * (popcount(bb(FPAWN))   - popcount(bb(EPAWN)))   +
    300 * (popcount(bb(FKNIGHT)) - popcount(bb(EKNIGHT))) +
    300 * (popcount(bb(FBISHOP)) - popcount(bb(EBISHOP))) +
    500 * (popcount(bb(FROOK))   - popcount(bb(EROOK)))   +
    900 * (popcount(bb(FQUEEN))  - popcount(bb(EQUEEN)));
}

template<Color Us>
int midgame()
{
  int score = material_count<Us>();

  constexpr Color Them         = !Us;
  constexpr Piece FriendlyPawn = make_piece(Us,   PAWN);
  constexpr Piece EnemyPawn    = make_piece(Them, PAWN);
  constexpr Piece FriendlyKing = make_piece(Us,   KING);
  constexpr Piece EnemyKing    = make_piece(Them, KING);

  score += king_safety<Us>(lsb(bb(FriendlyKing)), bb(FriendlyPawn)) - king_safety<Them>(lsb(bb(EnemyKing)), bb(EnemyPawn));

  constexpr Bitboard Rank567 = Us == WHITE ? RANK_5 | RANK_6 | RANK_7 : RANK_2 | RANK_3 | RANK_4;
  constexpr Bitboard Rank234 = Us == WHITE ? RANK_2 | RANK_3 | RANK_4 : RANK_5 | RANK_6 | RANK_7;

  score += 4 * (popcount(bb(FriendlyPawn) & Rank567) -  popcount(bb(EnemyPawn) & Rank234));

  for (PieceType pt : { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING })
  {
    for (Bitboard b = bitboards[make_piece(Us, pt)]; b; pop_lsb(b))
      score += square_score<Us>(pt, lsb(b));
    for (Bitboard b = bitboards[make_piece(Them, pt)]; b; pop_lsb(b))
      score -= square_score<Them>(pt, lsb(b));
  }

  return score;
}

template<Color Us>
int endgame()
{
  int score = material_count<Us>();

  constexpr Color Them         = !Us;
  constexpr Piece FriendlyPawn = make_piece(Us,   PAWN);
  constexpr Piece EnemyPawn    = make_piece(Them, PAWN);
  constexpr Piece FriendlyKing = make_piece(Us,   KING);
  constexpr Piece EnemyKing    = make_piece(Them, KING);

  score += end_king_squares[lsb(bb(FriendlyKing))];
  score -= end_king_squares[lsb(bb(EnemyKing   ))];

  constexpr Bitboard RANK2 = Us == WHITE ? RANK_2 : RANK_7;
  constexpr Bitboard RANK3 = Us == WHITE ? RANK_3 : RANK_6;
  constexpr Bitboard RANK4 = Us == WHITE ? RANK_4 : RANK_5;
  constexpr Bitboard RANK5 = Us == WHITE ? RANK_5 : RANK_4;
  constexpr Bitboard RANK6 = Us == WHITE ? RANK_6 : RANK_3;
  constexpr Bitboard RANK7 = Us == WHITE ? RANK_7 : RANK_2;

  score += 10 * popcount(bb(FriendlyPawn) & RANK4);
  score -= 10 * popcount(bb(EnemyPawn)    & RANK5);
  score += 20 * popcount(bb(FriendlyPawn) & RANK5);
  score -= 20 * popcount(bb(EnemyPawn)    & RANK4);
  score += 50 * popcount(bb(FriendlyPawn) & RANK6);
  score -= 50 * popcount(bb(EnemyPawn)    & RANK3);
  score += 90 * popcount(bb(FriendlyPawn) & RANK7);
  score -= 90 * popcount(bb(EnemyPawn)    & RANK2);

  return score;
}

template<Color Us>
int mopup()
{
  int score = material_count<Us>();

  constexpr Piece FriendlyKing = make_piece(Us, KING);
  constexpr Piece EnemyKing = make_piece(!Us, KING);

  if (score) {
    score += distance_from_center(lsb(bb(EnemyKing))) * 10;
    score += (14 - square_distance(lsb(bb(FriendlyKing)),lsb(bb(EnemyKing)))) * 4;
  } else {
    score -= distance_from_center(lsb(bb(FriendlyKing))) * 10;
    score -= (14 - square_distance(lsb(bb(FriendlyKing)),lsb(bb(EnemyKing)))) * 4;
  }

  return score;
}

template<Color Perspective>int static_eval()
  { return Position::midgame() ? midgame<Perspective>() : Position::endgame() ? endgame<Perspective>() : mopup<Perspective>(); }

#endif
