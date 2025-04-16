
#ifndef BITBOARD_H
#define BITBOARD_H

#include <cmath>
#include <immintrin.h>
#include <string>

#include "types.h"

#define pext(b, m) _pext_u64(b, m)
#define popcount(b) _mm_popcnt_u64(b)
#define lsb(b) _tzcnt_u64(b)

namespace Bitboards { void init(); }

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
inline uint8_t castle_masks[COLOR_NB][1 << 5];

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

inline Bitboard align_mask(Square ksq, Square pinned) {
    return AlignMask[ksq][pinned];
}

inline Bitboard main_diag(Square s) {
    return MainDiag[s];
}

inline Bitboard anti_diag(Square s) {
    return AntiDiag[s];
}

inline Bitboard file_bb(Square s) {
    return FileBB[s];
}

inline Bitboard double_check(Square ksq) {
    return DoubleCheck[ksq];
}

inline Bitboard check_ray(Square ksq, Square checker) {
    return CheckRay[ksq][checker];
}

constexpr Bitboard square_bb(Square s) {
    return 1ull << s;
}

template<typename... squares>
inline constexpr Bitboard square_bb(Square sq, squares... sqs) {
    return square_bb(sq) | square_bb(sqs...);
}

inline Bitboard rank_bb(Square s) {
    return RANK_1 << 8 * (s / 8);
}

inline std::string to_string(Bitboard b)
{
    std::string l = "+---+---+---+---+---+---+---+---+\n", s = l;

    for (Bitboard bit = square_bb(A8); bit; bit >>= 1)
    {
        s += (bit & b) ? "| @ " : "|   ";

        if (bit & FILE_H)
            s += "|\n" + l;
    }

    return s + "\n";
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

    if (d == NORTH || d == SOUTH)
    {
        Bitboard m = 0;

        while (is_ok(s += d))
            m |= rank_bb(s);

        return m;
    }
    else
    {
        Bitboard r = rank_bb(s), m = 0;

        while (square_bb(s += d) & r)
            m |= FILE_H << s % 8;

        return m;
    }
}

inline void clear_lsb(Bitboard& b) {
    b = _blsr_u64(b);
}

inline uint64_t more_than_one(Bitboard b) {
    return _blsr_u64(b);
}

inline Bitboard knight_attacks(Square sq) {
    return KnightAttacks[sq];
}

Bitboard attacks_bb(PieceType pt, Square sq, Bitboard occupied);

inline Bitboard xray_bb(PieceType pt, Square sq, Bitboard occupied) {
    return attacks_bb(pt, sq, occupied ^ attacks_bb(pt, sq, occupied) & occupied);
}

inline Bitboard king_attacks(Square sq) {
    return KingAttacks[sq];
}

template<Color C>
constexpr Bitboard pawn_attacks(Square sq) {
    return PawnAttacks[C][sq];
}

template<Color C>
constexpr Bitboard pawn_attacks(Bitboard pawns)
{
    if constexpr (C == WHITE) return shift<NORTH_EAST>(pawns) | shift<NORTH_WEST>(pawns);
    else                      return shift<SOUTH_WEST>(pawns) | shift<SOUTH_EAST>(pawns);
}

inline void toggle_square(Bitboard& b, Square s) {
    b ^= 1ull << s;
}

inline Bitboard generate_occupancy(Bitboard mask, int permutation) {
    return _pdep_u64(permutation, mask);
}

inline int square_distance(Square a, Square b) {
    return SquareDistance[a][b];
}

inline int file_distance(Square a, Square b) {
    return std::abs((a % 8) - (b % 8));
}

inline int rank_distance(Square a, Square b) {
    return std::abs((a / 8) - (b / 8));
}

inline Bitboard safe_step(Square s, int step)
{
    Square to = s + step;
    return (is_ok(to) && square_distance(s, to) <= 2) ? square_bb(to) : 0;
}

#endif