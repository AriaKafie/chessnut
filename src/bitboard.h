
#ifndef BITBOARD_H
#define BITBOARD_H

#include <cmath>
#include <immintrin.h>
#include <string>

#include "types.h"

#define pext(b, m) _pext_u64(b, m)
#define popcount(b) _mm_popcnt_u64(b)
#define lsb(b) _tzcnt_u64(b)

#ifndef PEXT
typedef struct
{
    Bitboard *ptr;
    Bitboard  mask;
    Bitboard  magic;
    int       shift;
} Magic;

inline Magic magics[SQUARE_NB][2];
#endif

std::string to_string(Bitboard b);

namespace Bitboards { void init(); }

inline Bitboard pext_table[0x1a480];
#ifdef PEXT
inline Bitboard xray_table[0x1a480];
#endif

inline Bitboard bishop_masks[SQUARE_NB];
inline Bitboard rook_masks[SQUARE_NB];

inline int bishop_base[SQUARE_NB];
inline int rook_base[SQUARE_NB];

inline Bitboard DoubleCheck[SQUARE_NB];
inline Bitboard KnightAttacks[SQUARE_NB];
inline Bitboard KingAttacks[SQUARE_NB];
inline Bitboard PawnAttacks[COLOR_NB][SQUARE_NB];
inline Bitboard CheckRay[SQUARE_NB][SQUARE_NB];
inline Bitboard AlignMask[SQUARE_NB][SQUARE_NB];
inline Bitboard MainDiag[SQUARE_NB];
inline Bitboard AntiDiag[SQUARE_NB];
inline Bitboard FileBB[SQUARE_NB];
inline uint8_t SquareDistance[SQUARE_NB][SQUARE_NB];
inline uint8_t CenterDistance[SQUARE_NB];
inline uint8_t CastleMasks[COLOR_NB][1 << 5];
inline Bitboard KingShield[COLOR_NB][SQUARE_NB];
inline int KingShieldScores[COLOR_NB][SQUARE_NB][1 << 6];

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

template<Direction D>
constexpr Bitboard shift(Bitboard bb)
{
    if constexpr (D == NORTH)       return  bb << 8;
    if constexpr (D == NORTH_EAST)  return (bb & NOT_FILE_H) << 7;
    if constexpr (D == EAST)        return  bb >> 1;
    if constexpr (D == SOUTH_EAST)  return (bb & NOT_FILE_H) >> 9;
    if constexpr (D == SOUTH)       return  bb >> 8;
    if constexpr (D == SOUTH_WEST)  return (bb & NOT_FILE_A) >> 7;
    if constexpr (D == WEST)        return  bb << 1;
    if constexpr (D == NORTH_WEST)  return (bb & NOT_FILE_A) << 9;
    if constexpr (D == NORTH+NORTH) return  bb << 16;
    if constexpr (D == SOUTH+SOUTH) return  bb >> 16;
}

template<Direction D>
constexpr Bitboard shift_unsafe(Bitboard bb)
{
    if constexpr (D == NORTH)       return  bb << 8;
    if constexpr (D == NORTH_EAST)  return  bb << 7;
    if constexpr (D == EAST)        return  bb >> 1;
    if constexpr (D == SOUTH_EAST)  return  bb >> 9;
    if constexpr (D == SOUTH)       return  bb >> 8;
    if constexpr (D == SOUTH_WEST)  return  bb >> 7;
    if constexpr (D == WEST)        return  bb << 1;
    if constexpr (D == NORTH_WEST)  return  bb << 9;
    if constexpr (D == NORTH+NORTH) return  bb << 16;
    if constexpr (D == SOUTH+SOUTH) return  bb >> 16;
}

inline Bitboard distance_from_center(Square s) {
    return CenterDistance[s];
}

inline int square_distance(Square a, Square b) {
    return SquareDistance[a][b];
}

inline Bitboard align_mask(Square s1, Square s2) {
    return AlignMask[s1][s2];
}

inline Bitboard main_diag(Square sq) {
    return MainDiag[sq];
}

inline Bitboard anti_diag(Square sq) {
    return AntiDiag[sq];
}

inline Bitboard file_bb(Square sq) {
    return FileBB[sq];
}

inline Bitboard double_check(Square sq) {
    return DoubleCheck[sq];
}

inline Bitboard check_ray(Square ksq, Square checker) {
    return CheckRay[ksq][checker];
}

constexpr Bitboard square_bb(Square sq) {
    return 1ull << sq;
}

template<typename... squares>
inline constexpr Bitboard square_bb(Square sq, squares... sqs) {
    return square_bb(sq) | square_bb(sqs...);
}

inline Bitboard rank_bb(Square s) {
    return RANK_1 << 8 * (s / 8);
}

inline Bitboard safe_step(Square s, int step)
{
    Square to = s + step;
    return (is_ok(to) && square_distance(s, to) <= 2) ? square_bb(to) : 0;
}

inline Bitboard mask(Square s, Direction d)
{
    switch (d) 
    {
        case NORTH_EAST: return mask(s, NORTH) & mask(s, EAST);
        case SOUTH_EAST: return mask(s, SOUTH) & mask(s, EAST);
        case SOUTH_WEST: return mask(s, SOUTH) & mask(s, WEST);
        case NORTH_WEST: return mask(s, NORTH) & mask(s, WEST);
    }

    Bitboard m = 0;

    while (safe_step(s, d) && is_ok(s += d))
        m |= d == NORTH || d == SOUTH ? rank_bb(s) : FILE_H << s % 8;

    return m;
}

inline void clear_lsb(Bitboard& b) {
    b = _blsr_u64(b);
}

inline bool more_than_one(Bitboard b) {
    return _blsr_u64(b);
}

inline Bitboard knight_attacks(Square sq) {
    return KnightAttacks[sq];
}

inline Bitboard bishop_attacks(Square sq, Bitboard occupied) {
#ifdef PEXT
    return pext_table[bishop_base[sq] + pext(occupied, bishop_masks[sq])];
#else
    occupied &=  magics[sq][0].mask;
    occupied *=  magics[sq][0].magic;
    occupied >>= magics[sq][0].shift;
    return       magics[sq][0].ptr[occupied];
#endif
}

inline Bitboard bishop_xray(Square sq, Bitboard occupied) {
#ifdef PEXT
    return xray_table[bishop_base[sq] + pext(occupied, bishop_masks[sq])];
#else
    return bishop_attacks(sq, occupied ^ bishop_attacks(sq, occupied) & occupied);
#endif
}

inline Bitboard rook_attacks(Square sq, Bitboard occupied) {
#ifdef PEXT
    return pext_table[rook_base[sq] + pext(occupied, rook_masks[sq])];
#else
    occupied &=  magics[sq][1].mask;
    occupied *=  magics[sq][1].magic;
    occupied >>= magics[sq][1].shift;
    return       magics[sq][1].ptr[occupied];
#endif
}

inline Bitboard rook_xray(Square sq, Bitboard occupied) {
#ifdef PEXT
    return xray_table[rook_base[sq] + pext(occupied, rook_masks[sq])];
#else
    return rook_attacks(sq, occupied ^ rook_attacks(sq, occupied) & occupied);
#endif
}

inline Bitboard queen_attacks(Square sq, Bitboard occupied) {
    return bishop_attacks(sq, occupied) | rook_attacks(sq, occupied);
}

inline Bitboard king_attacks(Square sq) {
    return KingAttacks[sq];
}

template<Color C>
constexpr Bitboard pawn_attacks(Square sq) {
    return PawnAttacks[C][sq];
}

template<Color C>
constexpr Bitboard pawn_attacks(Bitboard pawns) {
    return shift<relative_direction(C, NORTH_EAST)>(pawns) | shift<relative_direction(C, NORTH_WEST)>(pawns);
}

inline void toggle_square(Bitboard& b, Square s) {
    b ^= 1ull << s;
}

template<Color C>
int king_safety(Square ksq, Bitboard occ) {
    return KingShieldScores[C][ksq][pext(occ, KingShield[C][ksq])];
}

inline int file_distance(Square a, Square b) {
    return std::abs((a % 8) - (b % 8));
}

inline int rank_distance(Square a, Square b) {
    return std::abs((a / 8) - (b / 8));
}

#endif
