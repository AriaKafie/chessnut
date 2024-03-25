
#ifndef BITBOARD_H
#define BITBOARD_H

#include "types.h"

#include <immintrin.h>

#define pext(b, m) _pext_u64(b, m)
#define popcount(b) _mm_popcnt_u64(b)
#define lsb(b) _tzcnt_u64(b)

namespace Bitboards { void init(); }

namespace Magic {
  void search();
  void test_magic(Square sq, uint64_t magic);
}

inline Bitboard rookxray[102400];
inline Bitboard bishopxray[5248];
inline Bitboard rook_atk[102400];
inline Bitboard bishop_atk[5248];
inline Bitboard bishop_masks[SQUARE_NB];
inline Bitboard rook_masks[SQUARE_NB];
inline int rook_hash[SQUARE_NB];
inline int bishop_hash[SQUARE_NB];
inline Bitboard doublecheck[SQUARE_NB];
inline Bitboard knight_atk[SQUARE_NB];
inline Bitboard king_atk[SQUARE_NB];
inline Bitboard pawn_atk[COLOR_NB][SQUARE_NB];
inline Bitboard checkray[SQUARE_NB][SQUARE_NB];
inline Bitboard pinmask[SQUARE_NB][SQUARE_NB];
inline Bitboard main_diagonal[SQUARE_NB];
inline Bitboard anti_diagonal[SQUARE_NB];
inline Bitboard file[SQUARE_NB];
inline uint8_t square_dist[SQUARE_NB][SQUARE_NB];
inline uint8_t center_dist[SQUARE_NB];
inline int white_kingshield_scores[SQUARE_NB][1 << 6];
inline int black_kingshield_scores[SQUARE_NB][1 << 6];
inline uint8_t castle_pext[1 << 6];
inline Bitboard white_kingshield[SQUARE_NB];
inline Bitboard black_kingshield[SQUARE_NB];

constexpr Bitboard ALL_SQUARES = 0xffffffffffffffffull;
constexpr Bitboard FILE_A = 0x8080808080808080ull;
constexpr Bitboard FILE_B = FILE_A >> 1;
constexpr Bitboard FILE_C = FILE_A >> 2;
constexpr Bitboard FILE_D = FILE_A >> 3;
constexpr Bitboard FILE_E = FILE_A >> 4;
constexpr Bitboard FILE_F = FILE_A >> 5;
constexpr Bitboard FILE_G = FILE_A >> 6;
constexpr Bitboard FILE_H = FILE_A >> 7;
constexpr Bitboard NOT_FILE_A = ~FILE_A;
constexpr Bitboard NOT_FILE_H = ~FILE_H;

constexpr Bitboard RANK_1 = 0xffull;
constexpr Bitboard RANK_2 = RANK_1 << 8;
constexpr Bitboard RANK_3 = RANK_1 << 16;
constexpr Bitboard RANK_4 = RANK_1 << 24;
constexpr Bitboard RANK_5 = RANK_1 << 32;
constexpr Bitboard RANK_6 = RANK_1 << 40;
constexpr Bitboard RANK_7 = RANK_1 << 48;
constexpr Bitboard RANK_8 = RANK_1 << 56;
constexpr Bitboard NOT_RANK_2 = ~RANK_2;
constexpr Bitboard NOT_RANK_7 = ~RANK_7;

template<Direction D>
constexpr Bitboard shift(Bitboard bb) {
  if constexpr (D == NORTH)      return  bb << 8;
  if constexpr (D == NORTH_EAST) return (bb & NOT_FILE_H) << 7;
  if constexpr (D == EAST)       return  bb >> 1;
  if constexpr (D == SOUTH_EAST) return (bb & NOT_FILE_H) >> 9;
  if constexpr (D == SOUTH)      return  bb >> 8;
  if constexpr (D == SOUTH_WEST) return (bb & NOT_FILE_A) >> 7;
  if constexpr (D == WEST)       return  bb << 1;
  if constexpr (D == NORTH_WEST) return (bb & NOT_FILE_A) << 9;
  if constexpr (D == NORTHNORTH) return  bb << 16;
  if constexpr (D == SOUTHSOUTH) return  bb >> 16;
}

inline Bitboard distance_from_center(Square s) {
  return center_dist[s];
}

inline Bitboard pin_mask(Square ksq, Square pinned) {
  return pinmask[ksq][pinned];
}

inline Bitboard main_diag(Square s) {
  return main_diagonal[s];
}

inline Bitboard anti_diag(Square s) {
  return anti_diagonal[s];
}

inline Bitboard file_bb(Square s) {
  return file[s];
}

inline Bitboard double_check(Square ksq) {
  return doublecheck[ksq];
}

inline Bitboard check_ray(Square ksq, Square checker) {
  return checkray[ksq][checker];
}

constexpr Bitboard square_bb(Square s) {
  return 1ull << s;
}

template<typename... squares>
inline constexpr Bitboard square_bb(Square sq, squares... sqs) {
  return square_bb(sq) | square_bb(sqs...);
}

inline Bitboard rank_of(Square s) {
  return RANK_1 << 8 * (s / 8);
}

inline Bitboard file_of(Square s) {
  return FILE_H << (s % 8);
}

inline Bitboard mask(Square s, Direction d) {
  switch (d) 
  {
    case NORTH_EAST: return mask(s, NORTH) & mask(s, EAST);
    case SOUTH_EAST: return mask(s, SOUTH) & mask(s, EAST);
    case SOUTH_WEST: return mask(s, SOUTH) & mask(s, WEST);
    case NORTH_WEST: return mask(s, NORTH) & mask(s, WEST);
  }
  if (d == NORTH || d == SOUTH) {
    Bitboard m = 0;
    while (is_ok(s += d))
      m |= rank_of(s);
    return m;
  }
  else {
    Bitboard r = rank_of(s);
    Bitboard m = 0;
    while (square_bb(s += d) & r)
      m |= FILE_H << (s % 8);
    return m;
  }
}

inline void pop_lsb(Bitboard& b) {
  b = _blsr_u64(b);
}

inline uint64_t more_than_one(Bitboard b) {
  return _blsr_u64(b);
}

inline Bitboard knight_attacks(Square sq) {
  return knight_atk[sq];
}

inline Bitboard bishop_attacks(Square sq, Bitboard occupied) {
  return bishop_atk[bishop_hash[sq] + pext(occupied, bishop_masks[sq])];
}

inline Bitboard bishop_xray(Square sq, Bitboard occupied) {
  return bishopxray[bishop_hash[sq] + pext(occupied, bishop_masks[sq])];
}

inline Bitboard rook_attacks(Square sq, Bitboard occupied) {
  return rook_atk[rook_hash[sq] + pext(occupied, rook_masks[sq])];
}

inline Bitboard rook_xray(Square sq, Bitboard occupied) {
  return rookxray[rook_hash[sq] + pext(occupied, rook_masks[sq])];
}

inline Bitboard queen_attacks(Square sq, Bitboard occupied) {
  return bishop_atk[bishop_hash[sq] + pext(occupied, bishop_masks[sq])] | rook_atk[rook_hash[sq] + pext(occupied, rook_masks[sq])];
}

inline Bitboard king_attacks(Square sq) {
  return king_atk[sq];
}

template<Color C>
constexpr Bitboard pawn_attacks(Square sq) {
  return pawn_atk[C][sq];
}

template<Color C>
constexpr Bitboard pawn_attacks(Bitboard pawns) {
  if constexpr (C == WHITE)
    return shift<NORTH_EAST>(pawns) | shift<NORTH_WEST>(pawns);
  else 
    return shift<SOUTH_WEST>(pawns) | shift<SOUTH_EAST>(pawns);
}

inline void toggle_square(Bitboard& b, Square s) {
  b ^= 1ull << s;
}

inline Bitboard generate_occupancy(Bitboard mask, int permutation) {
  int bitcount = popcount(mask);
  Bitboard occupancy = 0;
  for (int bitpos = 0; bitpos < bitcount; bitpos++) {
    int lsb_index = lsb(mask);
    if (permutation & (1 << bitpos))
      occupancy |= 1ull << lsb_index;
    pop_lsb(mask);
  }
  return occupancy;
}

template<Color C>
int king_safety(Square ksq, Bitboard occ) {
  if constexpr (C == WHITE)
    return white_kingshield_scores[ksq][pext(occ, white_kingshield[ksq])];
  else
    return black_kingshield_scores[ksq][pext(occ, black_kingshield[ksq])];
}

inline uint8_t castling_pext(Bitboard occupied) {
  constexpr Bitboard mask = square_bb(A1, E1, H1, A8, E8, H8);
  return castle_pext[pext(occupied, mask)];
}

inline int square_distance(Square a, Square b) {
  return square_dist[a][b];
}

inline int file_distance(Square a, Square b) {
  return std::abs((a % 8) - (b % 8));
}

inline int rank_distance(Square a, Square b) {
  return std::abs((a / 8) - (b / 8));
}

inline Bitboard safe_step(Square s, int step) {
  Square to = s + step;
  return (is_ok(to) && square_distance(s, to) <= 2) ? square_bb(to) : 0;
}

#endif
