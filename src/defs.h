
#ifndef DEFS_H
#define DEFS_H

#include <intrin.h>

#define popcnt(x) _mm_popcnt_u64(x)
#define pext(x,m) _pext_u64(x,m)
#define tzcnt(x) _tzcnt_u64(x)
#define lzcnt(x) _lzcnt_u64(x)
#define blsr(x) _blsr_u64(x)

#define from_sq(move) (move & 0x3f)
#define to_sq(move) ((move >> 6) & 0x3f)
#define type_of(move) (move >> 12)

#define BISHOP_ATTACKS(square,occupancy) \
	Lookup::bishop_attacks[Lookup::bishop_hash[square] + _pext_u64(occupancy, Lookup::bishop_masks[square])]
#define ROOK_ATTACKS(square,occupancy) \
	Lookup::rook_attacks[Lookup::rook_hash[square] + _pext_u64(occupancy, Lookup::rook_masks[square])]
#define QUEEN_ATTACKS(square,occupancy) \
	(Lookup::rook_attacks[Lookup::rook_hash[square] + _pext_u64(occupancy, Lookup::rook_masks[square])] | \
	Lookup::bishop_attacks[Lookup::bishop_hash[square] + _pext_u64(occupancy, Lookup::bishop_masks[square])])
#define WHITE_KING_SAFETY(square,pawns) \
	Lookup::white_pawnshield_scores[square][_pext_u64(pawns,Lookup::white_pawnshield[square])]
#define BLACK_KING_SAFETY(square,pawns) \
	Lookup::black_pawnshield_scores[square][_pext_u64(pawns,Lookup::black_pawnshield[square])]

#define promotion_flag 0x1000
#define ep_flag 0x2000
#define scastle_flag 0x3000
#define lcastle_flag 0x4000
#define lsb_6 0x3f
#define MAX_INT 2147483647
#define MIN_INT -2147483648
#define MAX_LONG 0xffffffffffffffffull

#define RANK_567 0X00ffffff00000000ull
#define RANK_234 0x00000000ffffff00ull
#define not_2_or_h 0xfefefefefefe00feull
#define not_2_or_a 0x7f7f7f7f7f7f007full
#define not_7_or_h 0xfe00fefefefefefeull
#define not_7_or_a 0x7f007f7f7f7f7f7full
#define second_rank 0xff00ull
#define third_rank 0xff0000ull
#define fourth_rank 0xff000000ull
#define fifth_rank 0xff00000000ull
#define sixth_rank 0xff0000000000ull
#define seventh_rank 0xff000000000000ull
#define afile 0x8080808080808080ull
#define bfile 0x4040404040404040ull
#define cfile 0x2020202020202020ull
#define dfile 0x1010101010101010ull
#define efile 0x808080808080808ull
#define ffile 0x404040404040404ull
#define gfile 0x202020202020202ull
#define hfile 0x101010101010101ull
#define NOT_FILE_A 0x7f7f7f7f7f7f7f7full
#define NOT_FILE_H 0xfefefefefefefefeull

#define H1 0x1ull
#define E1 0x8ull
#define A1 0x80ull
#define H8 0x100000000000000ull
#define E8 0x800000000000000ull
#define A8 0x8000000000000000ull

#define Qkq 0b0111
#define Kkq 0b1011
#define KQq 0b1101
#define KQk 0b1110
#define kq  0b0011
#define KQ  0b1100

#define WHITE_PAWN   0
#define WHITE_KNIGHT 1
#define WHITE_BISHOP 2
#define WHITE_ROOK   3
#define WHITE_QUEEN  4
#define WHITE_KING   5
#define BLACK_PAWN   6
#define BLACK_KNIGHT 7
#define BLACK_BISHOP 8
#define BLACK_ROOK   9
#define BLACK_QUEEN  10
#define BLACK_KING   11
#define NULLPIECE    12

#define NULLMOVE     0
#define STANDARD     0
#define PROMOTION    1
#define ENPASSANT    2
#define SHORTCASTLE  3
#define LONGCASTLE   4

#define WPsquares_ten 0xff000000000000ull
#define WPsquares_six 0x180000000000ull
#define WPsquares_five 0x1800000000ull
#define WPsquares_four 0x24001800000000ull
#define WPsquares_two 0xc32400006600ull
#define WPsquares_one 0xc300998100ull
#define WPsquares_n_one 0x420000ull
#define WPsquares_n_two 0x240000ull
#define WPsquares_n_four 0x1800ull

#endif
