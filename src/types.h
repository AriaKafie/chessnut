
#ifndef TYPES_H
#define TYPES_H

#include <cstdint>

#ifdef _MSC_VER
  #define ForceInline __forceinline
#elif defined(__clang__)
  #define ForceInline __attribute__((always_inline))
#endif

//#define ForceInline

typedef uint64_t Bitboard;
typedef uint32_t Move;
typedef uint8_t  Piece;
typedef uint8_t  PieceType;
typedef int      Square;
typedef int      Color;
typedef int      MoveType;
typedef int      Direction;

constexpr int MAX_PLY  = 128;
constexpr int INFINITE = 0x7fffffff;

struct StateInfo {
    //Color    side_to_move;
    uint8_t  castling_rights;
    uint64_t key;
    Square   ep_sq;
    Piece    captured;
};

enum GamePhase { MIDGAME, ENDGAME, MOPUP };

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

enum PieceTypes : uint8_t {
  NO_PIECE_TYPE,
  PAWN = 2, KNIGHT, BISHOP, ROOK, QUEEN, KING,
  PIECE_TYPE_NB = 6
};

enum Pieces : uint8_t {
  NO_PIECE,
  W_PAWN = PAWN,     W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
  B_PAWN = PAWN + 8, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING
};

enum Colors { WHITE, BLACK, COLOR_NB = 2 };

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

constexpr Piece make_piece(Color c, PieceType pt) {
  return pt + (c << 3);
}

constexpr PieceType type_of(Piece p) {
  return p & 7;
}

constexpr Color color_of(Piece p) {
  return p >> 3;
}

ForceInline constexpr Move make_move(Square from, Square to) {
  return from + (to << 6);
}

template<MoveType T>
ForceInline constexpr Move make_move(Square from, Square to) {
  return T + from + (to << 6);
}

enum Moves {
  NO_MOVE,
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

constexpr uint32_t score_of(Move m) {
  return m & 0xffff0000;
}

#endif
