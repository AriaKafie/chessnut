
#ifndef TYPES_H
#define TYPES_H

#include <cstdint>
#include <cmath>

#ifdef _MSC_VER
#define ForceInline __forceinline
#elif defined(__clang__)
#define ForceInline __attribute__((always_inline))
#endif

using Bitboard  = uint64_t;
using Square    = int;
using Move      = int;
using Piece     = int;
using PieceType = int;
using Color     = int;
using MoveType  = int;
using Direction = int;
using Rank      = int;

enum Files {
  FILE1,FILE2,FILE3,FILE4,
  FILE5,FILE6,FILE7,FILE8
};

enum Ranks {
  RANK1,RANK2,RANK3,RANK4,
  RANK5,RANK6,RANK7,RANK8
};

enum Squares {
  H1, G1, F1, E1, D1, C1, B1, A1,
  H2, G2, F2, E2, D2, C2, B2, A2,
  H3, G3, F3, E3, D3, C3, B3, A3,
  H4, G4, F4, E4, D4, C4, B4, A4,
  H5, G5, F5, E5, D5, C5, B5, A5,
  H6, G6, F6, E6, D6, C6, B6, A6,
  H7, G7, F7, E7, D7, C7, B7, A7,
  H8, G8, F8, E8, D8, C8, B8, A8,
  SQUARE_NB = 64
};

enum Pieces {
  W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
  B_PAWN, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING,
  NO_PIECE,
  PIECE_NB = 13
};

enum PieceTypes {
  PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING,
  NO_PIECE_TYPE,
  PIECE_TYPE_NB = 6
};

enum Colors {
  BLACK,
  WHITE,
  COLOR_NB = 2
};

enum MoveTypes {
  NORMAL,
  PROMOTION   = 1 << 12,
  ENPASSANT   = 2 << 12,
  SHORTCASTLE = 3 << 12,
  LONGCASTLE  = 4 << 12,
};

enum Directions {
  NORTH =  8,
  EAST  = -1,
  SOUTH = -8,
  WEST  =  1,
  NORTH_EAST = NORTH + EAST,
  SOUTH_EAST = SOUTH + EAST,
  NORTH_WEST = NORTH + WEST,
  SOUTH_WEST = SOUTH + WEST,
  NORTHNORTH = NORTH + NORTH,
  SOUTHSOUTH = SOUTH + SOUTH
};

inline Rank rank_of(Square s) {
  return s % 8;
}

template<Color C, PieceType P>
constexpr Piece piece() {
  if constexpr (C == WHITE)
    return P;
  else
    return P + B_PAWN;
}

template<Color C>
PieceType piece_type(Piece p) {
  if constexpr (C == WHITE)
    return p;
  else
    return p - B_PAWN;
}

template<Color C>
constexpr Color flip() {
  if constexpr (C == WHITE)
    return BLACK;
  return WHITE;
}

ForceInline inline Move make_move(Square from, Square to) {
  return from + (to << 6);
}

template<MoveType T>
ForceInline constexpr Move make_move(Square from, Square to) {
  return T + from + (to << 6);
}

enum Moves {
  NULLMOVE,
  W_SCASTLE = make_move<SHORTCASTLE>(E1, G1),
  W_LCASTLE = make_move<LONGCASTLE >(E1, C1),
  B_SCASTLE = make_move<SHORTCASTLE>(E8, G8),
  B_LCASTLE = make_move<LONGCASTLE >(E8, C8),
};

inline bool is_ok(Square s) {
  return s >= H1 && s <= A8; 
}

constexpr Square from_sq(Move m) {
  return m & 0x3f;
}

constexpr Square to_sq(Move m) {
  return (m >> 6) & 0x3f;
}

constexpr int from_to(Move m) {
  return m & 0xfff;
}

constexpr MoveType type_of(Move m) {
  return m & 0x7000;
}

constexpr int score_of(Move m) {
  return m & 0xffff0000;
}

#endif
