
#ifndef BITBOARD_H
#define BITBOARD_H

#include <cmath>
#include <string>

#include "types.h"

#define pext(b, m) _pext_u64(b, m)

#ifdef BMI
    #include <immintrin.h>

    #define popcount(b) _mm_popcnt_u64(b)
    #define lsb(b) _tzcnt_u64(b)
#else

inline const int index64[64] = {
    0,  1, 48,  2, 57, 49, 28,  3,
   61, 58, 50, 42, 38, 29, 17,  4,
   62, 55, 59, 36, 53, 51, 43, 22,
   45, 39, 33, 30, 24, 18, 12,  5,
   63, 47, 56, 27, 60, 41, 37, 16,
   54, 35, 52, 21, 44, 32, 23, 11,
   46, 26, 40, 15, 34, 20, 31, 10,
   25, 14, 19,  9, 13,  8,  7,  6
};

inline int lsb(Bitboard b) {
    return index64[((b & -b) * 0x03f79d71b4cb0a89ull) >> 58];
}

inline uint8_t popcnt16[1 << 16];

inline int popcount(Bitboard b) {
    union {
        Bitboard bb;
        uint16_t v[4];
    } u = {b};
    return popcnt16[u.v[0]] + popcnt16[u.v[1]] + popcnt16[u.v[2]] + popcnt16[u.v[3]];
}

inline Bitboard KingShieldMagics[COLOR_NB][SQUARE_NB];

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

#ifdef BMI
    inline Bitboard xray_table[0x1a480];

    inline Bitboard bishop_masks[SQUARE_NB];
    inline Bitboard rook_masks[SQUARE_NB];

    inline int bishop_base[SQUARE_NB];
    inline int rook_base[SQUARE_NB];
#endif

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
#ifdef BMI
    b = _blsr_u64(b);
#else
    b = b & (b - 1);
#endif
}

inline bool more_than_one(Bitboard b) {
#ifdef BMI
    return _blsr_u64(b);
#else
    return b & (b - 1);
#endif
}

inline Bitboard knight_attacks(Square sq) {
    return KnightAttacks[sq];
}

inline Bitboard bishop_attacks(Square sq, Bitboard occupied) {
#ifdef BMI
    return pext_table[bishop_base[sq] + pext(occupied, bishop_masks[sq])];
#else
    occupied &=  magics[sq][0].mask;
    occupied *=  magics[sq][0].magic;
    occupied >>= magics[sq][0].shift;
    return       magics[sq][0].ptr[occupied];
#endif
}

inline Bitboard bishop_xray(Square sq, Bitboard occupied) {
#ifdef BMI
    return xray_table[bishop_base[sq] + pext(occupied, bishop_masks[sq])];
#else
    return bishop_attacks(sq, occupied ^ bishop_attacks(sq, occupied) & occupied);
#endif
}

inline Bitboard rook_attacks(Square sq, Bitboard occupied) {
#ifdef BMI
    return pext_table[rook_base[sq] + pext(occupied, rook_masks[sq])];
#else
    occupied &=  magics[sq][1].mask;
    occupied *=  magics[sq][1].magic;
    occupied >>= magics[sq][1].shift;
    return       magics[sq][1].ptr[occupied];
#endif
}

inline Bitboard rook_xray(Square sq, Bitboard occupied) {
#ifdef BMI
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
#ifdef BMI
    return KingShieldScores[C][ksq][pext(occ, KingShield[C][ksq])];
#else
    occ &=  KingShield[C][ksq];
    occ *=  KingShieldMagics[C][ksq];
    occ >>= 58;
    return KingShieldScores[C][ksq][occ];
#endif
}

inline int file_distance(Square a, Square b) {
    return std::abs((a % 8) - (b % 8));
}

inline int rank_distance(Square a, Square b) {
    return std::abs((a / 8) - (b / 8));
}

#endif
