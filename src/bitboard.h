
#ifndef BITBOARD_H
#define BITBOARD_H

#include <cmath>
#include <string>

#include "types.h"

#ifdef BMI
    #include <immintrin.h>

    #define pext(b, m) _pext_u64(b, m)
    #define popcount(b) _mm_popcnt_u64(b)
    #define lsb(b) _tzcnt_u64(b)
#else

inline const int bitscan[64] {
    63, 0, 58, 1, 59, 19, 36, 2,
    60, 43, 31, 20, 54, 37, 3, 46,
    61, 34, 41, 44, 32, 8, 10, 21,
    55, 28, 38, 12, 15, 4, 23, 47,
    62, 57, 18, 35, 42, 30, 53, 45,
    33, 40, 7, 9, 27, 11, 14, 22,
    56, 17, 29, 52, 39, 6, 26, 13,
    16, 51, 5, 25, 50, 24, 49, 48,
};

inline int lsb(Bitboard b) {
    return bitscan[(b & -b) * 0x756e2f651a4fcc2ull >> 58];
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

typedef struct {
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
inline Bitboard Passers[COLOR_NB][4][1 << 10];
inline int KingShieldScores[COLOR_NB][SQUARE_NB][1 << 6];

constexpr Bitboard FILE_ABB = 0x8080808080808080ull;
constexpr Bitboard FILE_BBB = FILE_ABB >> 1;
constexpr Bitboard FILE_CBB = FILE_ABB >> 2;
constexpr Bitboard FILE_DBB = FILE_ABB >> 3;
constexpr Bitboard FILE_EBB = FILE_ABB >> 4;
constexpr Bitboard FILE_FBB = FILE_ABB >> 5;
constexpr Bitboard FILE_GBB = FILE_ABB >> 6;
constexpr Bitboard FILE_HBB = FILE_ABB >> 7;

constexpr Bitboard RANK_1BB = 0xffull;
constexpr Bitboard RANK_2BB = RANK_1BB << 8;
constexpr Bitboard RANK_3BB = RANK_1BB << 16;
constexpr Bitboard RANK_4BB = RANK_1BB << 24;
constexpr Bitboard RANK_5BB = RANK_1BB << 32;
constexpr Bitboard RANK_6BB = RANK_1BB << 40;
constexpr Bitboard RANK_7BB = RANK_1BB << 48;
constexpr Bitboard RANK_8BB = RANK_1BB << 56;

template<Direction D>
constexpr Bitboard shift(Bitboard bb)
{
    if constexpr (D == NORTH)       return  bb << 8;
    if constexpr (D == NORTH_EAST)  return (bb & ~FILE_HBB) << 7;
    if constexpr (D == EAST)        return  bb >> 1;
    if constexpr (D == SOUTH_EAST)  return (bb & ~FILE_HBB) >> 9;
    if constexpr (D == SOUTH)       return  bb >> 8;
    if constexpr (D == SOUTH_WEST)  return (bb & ~FILE_ABB) >> 7;
    if constexpr (D == WEST)        return  bb << 1;
    if constexpr (D == NORTH_WEST)  return (bb & ~FILE_ABB) << 9;
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
    return RANK_1BB << 8 * (s / 8);
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
        m |= d == NORTH || d == SOUTH ? rank_bb(s) : FILE_HBB << s % 8;

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
