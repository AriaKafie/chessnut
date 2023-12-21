
#ifndef BOARD_H
#define BOARD_H

#include "defs.h"
#include "gamestate.h"
#include "zobrist.h"
#include "bitboard.h"

#include <ammintrin.h>
#include <string>
#include <cstdint>
#include <iostream>

namespace Board {
	inline Piece pieces[SQUARE_NB];
	inline Bitboard bitboards[PIECE_NB];
	inline Bitboard white_pieces;
	inline Bitboard black_pieces;
}

inline PieceType piece_type(Square s) {
	return std::max(0, Board::pieces[s] - B_PAWN);
}

namespace Board {

inline constexpr int non_null[PIECE_NB] = { 1,1,1,1,1,1,1,1,1,1,1,1,0 };
inline Bitboard occupied() { return white_pieces | black_pieces; }

void make_legal(Move move);
void undo_legal(Move move, Piece capture);

inline bool white_king_in_check() {
	Bitboard BishopQueen = bitboards[B_BISHOP] | bitboards[B_QUEEN];
	Bitboard RookQueen   = bitboards[B_ROOK]   | bitboards[B_QUEEN];
	Square   ksq         = lsb(bitboards[W_KING]);
	return
		  (PawnAttacks<WHITE>(ksq)            & bitboards[B_PAWN])
		| (KnightAttacks     (ksq)            & bitboards[B_KNIGHT])
		| (BishopAttacks     (ksq,occupied()) & BishopQueen)
		| (RookAttacks       (ksq,occupied()) & RookQueen);
}

inline bool black_king_in_check() {
	Bitboard BishopQueen = bitboards[W_BISHOP] | bitboards[W_QUEEN];
	Bitboard RookQueen   = bitboards[W_ROOK]   | bitboards[W_QUEEN];
	Square ksq           = lsb(bitboards[B_KING]);
	return
		  (PawnAttacks<BLACK>(ksq)            & bitboards[W_PAWN])
		| (KnightAttacks     (ksq)            & bitboards[W_KNIGHT])
		| (BishopAttacks     (ksq,occupied()) & BishopQueen)
		| (RookAttacks       (ksq,occupied()) & RookQueen);
}

template<Color JustMoved, MoveType MoveType>
inline ForceInline void update_castling_rights() {

	#define bb bitboards
	using GameState::castling_rights;

	constexpr Bitboard WKING_START = square_bb(E1);
	constexpr Bitboard BKING_START = square_bb(E8);
	constexpr Bitboard WROOK_START = square_bb(A1, H1);
	constexpr Bitboard BROOK_START = square_bb(A8, H8);

	if constexpr (JustMoved == WHITE) {
		if constexpr (MoveType == NORMAL) {
			uint64_t key = (bb[W_KING] & WKING_START)
				         | (bb[W_ROOK] & WROOK_START)
				         |  bb[B_ROOK] | BKING_START;
			castling_rights &= CastlingPext(key);
		}
		else if constexpr (MoveType == PROMOTION) {
			uint64_t key =  bb[B_ROOK] | WROOK_START
				         | BKING_START | WKING_START;
			castling_rights &= CastlingPext(key);
		}
		else if constexpr (MoveType == SHORTCASTLE | MoveType == LONGCASTLE) {
			castling_rights &= 0b0011;
		}
	}
	else {
		if constexpr (MoveType == NORMAL) {
			uint64_t key = (bb[B_KING] & BKING_START)
				         | (bb[B_ROOK] & BROOK_START)
				         |  bb[W_ROOK] | WKING_START;
			castling_rights &= CastlingPext(key);
		}
		else if constexpr (MoveType == PROMOTION) {
			uint64_t key =  bb[W_ROOK] | BROOK_START
				         | BKING_START | WKING_START;
			castling_rights &= CastlingPext(key);
		}
		else if constexpr (MoveType == SHORTCASTLE | MoveType == LONGCASTLE) {
			castling_rights &= 0b1100;
		}
	}
	#undef bb
}

template<Color C, bool CapturesOnly>
void makemove(int move) {

	Square from = from_sq(move);
	Square to = to_sq(move);
	MoveType movetype = type_of(move);

	if constexpr (C == WHITE) {
		if constexpr (CapturesOnly) {
			switch (movetype) {
			case NORMAL:
				Zobrist::key ^= Zobrist::hash[pieces[from]][from];
				Zobrist::key ^= Zobrist::hash[pieces[to]][to];
				Zobrist::key ^= Zobrist::hash[pieces[from]][to];
				Zobrist::key ^= Zobrist::black_to_move;
				bitboards[pieces[to]] ^= square_bb(to);
				bitboards[pieces[from]] ^= square_bb(from);
				bitboards[pieces[from]] ^= square_bb(to);
				white_pieces ^= square_bb(from);
				white_pieces ^= square_bb(to);
				black_pieces ^= square_bb(to);
				pieces[to] = pieces[from];
				pieces[from] = NO_PIECE;
				return;
			case PROMOTION:
				Zobrist::key ^= Zobrist::hash[W_PAWN][from];
				Zobrist::key ^= Zobrist::hash[pieces[to]][to];
				Zobrist::key ^= Zobrist::hash[W_QUEEN][to];
				Zobrist::key ^= Zobrist::black_to_move;
				bitboards[pieces[to]] ^= 1ull << to;
				bitboards[W_PAWN] ^= 1ull << from;
				bitboards[W_QUEEN] ^= 1ull << to;
				white_pieces ^= 1ull << from;
				white_pieces ^= 1ull << to;
				black_pieces ^= 1ull << to;
				pieces[to] = W_QUEEN;
				pieces[from] = NO_PIECE;
				return;
			}
		}
		else {
			switch (movetype) {
			case NORMAL:
				Zobrist::key ^= Zobrist::hash[pieces[from]][from];
				Zobrist::key ^= Zobrist::hash[pieces[to]][to];
				Zobrist::key ^= Zobrist::hash[pieces[from]][to];
				Zobrist::key ^= Zobrist::black_to_move;
				bitboards[pieces[to]] ^= 1ull << to;
				bitboards[pieces[from]] ^= 1ull << from;
				bitboards[pieces[from]] ^= 1ull << to;
				white_pieces ^= 1ull << from;
				white_pieces ^= 1ull << to;
				black_pieces &= ~square_bb(to);
				pieces[to] = pieces[from];
				pieces[from] = NO_PIECE;
				update_castling_rights<WHITE, NORMAL>();
				return;
			case PROMOTION:
				Zobrist::key ^= Zobrist::hash[W_PAWN][from];
				Zobrist::key ^= Zobrist::hash[pieces[to]][to];
				Zobrist::key ^= Zobrist::hash[W_QUEEN][to];
				Zobrist::key ^= Zobrist::black_to_move;
				bitboards[pieces[to]] ^= 1ull << to;
				bitboards[W_PAWN] ^= 1ull << from;
				bitboards[W_QUEEN] ^= 1ull << to;
				white_pieces ^= 1ull << from;
				white_pieces ^= 1ull << to;
				black_pieces &= ~square_bb(to);
				pieces[to] = W_QUEEN;
				pieces[from] = NO_PIECE;
				update_castling_rights<WHITE, PROMOTION>();
				return;
			case SHORTCASTLE:
				bitboards[W_ROOK] ^= 0x5ull;
				bitboards[W_KING] ^= 0xaull;
				white_pieces ^= 0xfull;
				pieces[3] = NO_PIECE;
				pieces[0] = NO_PIECE;
				pieces[1] = W_KING;
				pieces[2] = W_ROOK;
				Zobrist::key ^= Zobrist::hash[W_KING][3];
				Zobrist::key ^= Zobrist::hash[W_ROOK][0];
				Zobrist::key ^= Zobrist::hash[W_KING][1];
				Zobrist::key ^= Zobrist::hash[W_ROOK][2];
				Zobrist::key ^= Zobrist::black_to_move;
				update_castling_rights<WHITE, SHORTCASTLE>();
				return;
			case LONGCASTLE:
				bitboards[W_ROOK] ^= 0x90ull;
				bitboards[W_KING] ^= 0x28ull;
				white_pieces ^= 0xb8ull;
				pieces[3] = NO_PIECE;
				pieces[7] = NO_PIECE;
				pieces[5] = W_KING;
				pieces[4] = W_ROOK;
				Zobrist::key ^= Zobrist::hash[W_KING][3];
				Zobrist::key ^= Zobrist::hash[W_ROOK][7];
				Zobrist::key ^= Zobrist::hash[W_KING][5];
				Zobrist::key ^= Zobrist::hash[W_ROOK][4];
				Zobrist::key ^= Zobrist::black_to_move;
				update_castling_rights<WHITE, LONGCASTLE>();
				return;
			case ENPASSANT:
				Zobrist::key ^= Zobrist::hash[W_PAWN][from];
				Zobrist::key ^= Zobrist::hash[B_PAWN][to - 8];
				Zobrist::key ^= Zobrist::hash[W_PAWN][to];
				Zobrist::key ^= Zobrist::black_to_move;
				bitboards[W_PAWN] ^= 1ull << from;
				bitboards[W_PAWN] ^= 1ull << to;
				bitboards[B_PAWN] ^= 1ull << (to - 8);
				white_pieces ^= 1ull << from;
				white_pieces ^= 1ull << to;
				black_pieces ^= 1ull << (to - 8);
				pieces[from] = NO_PIECE;
				pieces[to] = W_PAWN;
				pieces[to - 8] = NO_PIECE;
				return;
			}
		}
	}
	else {
		if constexpr (CapturesOnly) {
			switch (movetype) {
			case NORMAL:
				Zobrist::key ^= Zobrist::black_to_move;
				Zobrist::key ^= Zobrist::hash[pieces[from]][from];
				Zobrist::key ^= Zobrist::hash[pieces[to]][to];
				Zobrist::key ^= Zobrist::hash[pieces[from]][to];
				bitboards[pieces[to]] ^= 1ull << to;
				bitboards[pieces[from]] ^= 1ull << from;
				bitboards[pieces[from]] ^= 1ull << to;
				black_pieces ^= 1ull << from;
				black_pieces ^= 1ull << to;
				white_pieces ^= 1ull << to;
				pieces[to] = pieces[from];
				pieces[from] = NO_PIECE;
				return;
			case PROMOTION:
				Zobrist::key ^= Zobrist::black_to_move;
				Zobrist::key ^= Zobrist::hash[B_PAWN][from];
				Zobrist::key ^= Zobrist::hash[pieces[to]][to];
				Zobrist::key ^= Zobrist::hash[B_QUEEN][to];
				bitboards[pieces[to]] ^= 1ull << to;
				bitboards[B_PAWN] ^= 1ull << from;
				bitboards[B_QUEEN] ^= 1ull << to;
				black_pieces ^= 1ull << from;
				black_pieces ^= 1ull << to;
				white_pieces ^= 1ull << to;
				pieces[to] = B_QUEEN;
				pieces[from] = NO_PIECE;
				return;
			}
		}
		else {
			switch (movetype) {
			case NORMAL:
				Zobrist::key ^= Zobrist::black_to_move;
				Zobrist::key ^= Zobrist::hash[pieces[from]][from];
				Zobrist::key ^= Zobrist::hash[pieces[to]][to];
				Zobrist::key ^= Zobrist::hash[pieces[from]][to];
				bitboards[pieces[to]] ^= 1ull << to;
				bitboards[pieces[from]] ^= 1ull << from;
				bitboards[pieces[from]] ^= 1ull << to;
				black_pieces ^= 1ull << from;
				black_pieces ^= 1ull << to;
				white_pieces &= ~square_bb(to);
				pieces[to] = pieces[from];
				pieces[from] = NO_PIECE;
				update_castling_rights<BLACK, NORMAL>();
				return;
			case PROMOTION:
				Zobrist::key ^= Zobrist::black_to_move;
				Zobrist::key ^= Zobrist::hash[B_PAWN][from];
				Zobrist::key ^= Zobrist::hash[pieces[to]][to];
				Zobrist::key ^= Zobrist::hash[B_QUEEN][to];
				bitboards[pieces[to]] ^= 1ull << to;
				bitboards[B_PAWN] ^= 1ull << from;
				bitboards[B_QUEEN] ^= 1ull << to;
				black_pieces ^= 1ull << from;
				black_pieces ^= 1ull << to;
				white_pieces &= ~square_bb(to);
				pieces[to] = B_QUEEN;
				pieces[from] = NO_PIECE;
				update_castling_rights<BLACK, PROMOTION>();
				return;
			case SHORTCASTLE:
				bitboards[B_ROOK] ^= 0x500000000000000ull;
				bitboards[B_KING] ^= 0xa00000000000000ull;
				black_pieces ^= 0xf00000000000000ull;
				pieces[59] = NO_PIECE;
				pieces[56] = NO_PIECE;
				pieces[57] = B_KING;
				pieces[58] = B_ROOK;
				Zobrist::key ^= Zobrist::black_to_move;
				Zobrist::key ^= Zobrist::hash[B_KING][59];
				Zobrist::key ^= Zobrist::hash[B_ROOK][56];
				Zobrist::key ^= Zobrist::hash[B_KING][57];
				Zobrist::key ^= Zobrist::hash[B_ROOK][58];
				update_castling_rights<BLACK, SHORTCASTLE>();
				return;
			case LONGCASTLE:
				bitboards[B_ROOK] ^= 0x9000000000000000ull;
				bitboards[B_KING] ^= 0x2800000000000000ull;
				black_pieces ^= 0xb800000000000000ull;
				pieces[59] = NO_PIECE;
				pieces[63] = NO_PIECE;
				pieces[61] = B_KING;
				pieces[60] = B_ROOK;
				Zobrist::key ^= Zobrist::black_to_move;
				Zobrist::key ^= Zobrist::hash[B_KING][59];
				Zobrist::key ^= Zobrist::hash[B_ROOK][63];
				Zobrist::key ^= Zobrist::hash[B_KING][61];
				Zobrist::key ^= Zobrist::hash[B_ROOK][60];
				update_castling_rights<BLACK, LONGCASTLE>();
				return;
			case ENPASSANT:
				Zobrist::key ^= Zobrist::black_to_move;
				Zobrist::key ^= Zobrist::hash[B_PAWN][from];
				Zobrist::key ^= Zobrist::hash[W_PAWN][to + 8];
				Zobrist::key ^= Zobrist::hash[B_PAWN][to];
				bitboards[B_PAWN] ^= 1ull << from;
				bitboards[B_PAWN] ^= 1ull << to;
				bitboards[W_PAWN] ^= 1ull << (to + 8);
				black_pieces ^= 1ull << from;
				black_pieces ^= 1ull << to;
				white_pieces ^= 1ull << (to + 8);
				pieces[from] = NO_PIECE;
				pieces[to] = B_PAWN;
				pieces[to + 8] = NO_PIECE;
				return;
			}
		}
	}
}

template<Color C, bool CapturesOnly>
ForceInline void undomove(int move, Piece capture) {

	int from = from_sq(move);
	int to = to_sq(move);
	int movetype = type_of(move);

	if constexpr (C == WHITE) {
		if constexpr (CapturesOnly) {
			switch (movetype) {
			case NORMAL:
				Zobrist::key ^= Zobrist::black_to_move;
				Zobrist::key ^= Zobrist::hash[pieces[to]][to];
				Zobrist::key ^= Zobrist::hash[pieces[to]][from];
				Zobrist::key ^= Zobrist::hash[capture][to];
				bitboards[pieces[to]] ^= 1ull << to;
				bitboards[pieces[to]] ^= 1ull << from;
				bitboards[capture] ^= 1ull << to;
				white_pieces ^= 1ull << to;
				white_pieces ^= 1ull << from;
				black_pieces ^= 1ull << to;
				pieces[from] = pieces[to];
				pieces[to] = capture;
				return;
			case PROMOTION:
				Zobrist::key ^= Zobrist::black_to_move;
				Zobrist::key ^= Zobrist::hash[W_QUEEN][to];
				Zobrist::key ^= Zobrist::hash[W_PAWN][from];
				Zobrist::key ^= Zobrist::hash[capture][to];
				bitboards[W_QUEEN] ^= 1ull << to;
				bitboards[W_PAWN] ^= 1ull << from;
				bitboards[capture] ^= 1ull << to;
				white_pieces ^= 1ull << to;
				white_pieces ^= 1ull << from;
				black_pieces ^= 1ull << to;
				pieces[to] = capture;
				pieces[from] = W_PAWN;
				return;
			}
		}
		else {
			switch (movetype) {
			case NORMAL:
				Zobrist::key ^= Zobrist::black_to_move;
				Zobrist::key ^= Zobrist::hash[pieces[to]][to];
				Zobrist::key ^= Zobrist::hash[pieces[to]][from];
				Zobrist::key ^= Zobrist::hash[capture][to];
				bitboards[pieces[to]] ^= 1ull << to;
				bitboards[pieces[to]] ^= 1ull << from;
				bitboards[capture] ^= 1ull << to;
				white_pieces ^= 1ull << to;
				white_pieces ^= 1ull << from;
				black_pieces ^= square_bb(to) * non_null[capture];
				pieces[from] = pieces[to];
				pieces[to] = capture;
				return;
			case PROMOTION:
				Zobrist::key ^= Zobrist::black_to_move;
				Zobrist::key ^= Zobrist::hash[W_QUEEN][to];
				Zobrist::key ^= Zobrist::hash[W_PAWN][from];
				Zobrist::key ^= Zobrist::hash[capture][to];
				bitboards[W_QUEEN] ^= 1ull << to;
				bitboards[W_PAWN] ^= 1ull << from;
				bitboards[capture] ^= 1ull << to;
				white_pieces ^= 1ull << to;
				white_pieces ^= 1ull << from;
				black_pieces ^= square_bb(to) * non_null[capture];
				pieces[to] = capture;
				pieces[from] = W_PAWN;
				return;
			case SHORTCASTLE:
				bitboards[W_ROOK] ^= 0x5ull;
				bitboards[W_KING] ^= 0xaull;
				white_pieces ^= 0xfull;
				pieces[3] = W_KING;
				pieces[0] = W_ROOK;
				pieces[1] = NO_PIECE;
				pieces[2] = NO_PIECE;
				Zobrist::key ^= Zobrist::black_to_move;
				Zobrist::key ^= Zobrist::hash[W_KING][1];
				Zobrist::key ^= Zobrist::hash[W_ROOK][2];
				Zobrist::key ^= Zobrist::hash[W_KING][3];
				Zobrist::key ^= Zobrist::hash[W_ROOK][0];
				return;
			case LONGCASTLE:
				bitboards[W_ROOK] ^= 0x90ull;
				bitboards[W_KING] ^= 0x28ull;
				white_pieces ^= 0xb8ull;
				pieces[3] = W_KING;
				pieces[7] = W_ROOK;
				pieces[5] = NO_PIECE;
				pieces[4] = NO_PIECE;
				Zobrist::key ^= Zobrist::black_to_move;
				Zobrist::key ^= Zobrist::hash[W_KING][5];
				Zobrist::key ^= Zobrist::hash[W_ROOK][4];
				Zobrist::key ^= Zobrist::hash[W_KING][3];
				Zobrist::key ^= Zobrist::hash[W_ROOK][7];
				return;
			case ENPASSANT:
				Zobrist::key ^= Zobrist::black_to_move;
				Zobrist::key ^= Zobrist::hash[W_PAWN][to];
				Zobrist::key ^= Zobrist::hash[W_PAWN][from];
				Zobrist::key ^= Zobrist::hash[B_PAWN][to - 8];
				bitboards[W_PAWN] ^= 1ull << to;
				bitboards[W_PAWN] ^= 1ull << from;
				bitboards[B_PAWN] ^= 1ull << (to - 8);
				white_pieces ^= 1ull << to;
				white_pieces ^= 1ull << from;
				black_pieces |= bitboards[B_PAWN];
				pieces[to] = NO_PIECE;
				pieces[from] = W_PAWN;
				pieces[to - 8] = B_PAWN;
				return;
			}
		}
	}
	else {
		if constexpr (CapturesOnly) {
			switch (movetype) {
			case NORMAL:
				Zobrist::key ^= Zobrist::hash[pieces[to]][to];
				Zobrist::key ^= Zobrist::hash[pieces[to]][from];
				Zobrist::key ^= Zobrist::hash[capture][to];
				Zobrist::key ^= Zobrist::black_to_move;
				bitboards[pieces[to]] ^= 1ull << to;
				bitboards[pieces[to]] ^= 1ull << from;
				bitboards[capture] ^= 1ull << to;
				black_pieces ^= 1ull << to;
				black_pieces ^= 1ull << from;
				white_pieces ^= 1ull << to;
				pieces[from] = pieces[to];
				pieces[to] = capture;
				return;
			case PROMOTION:
				Zobrist::key ^= Zobrist::hash[B_QUEEN][to];
				Zobrist::key ^= Zobrist::hash[B_PAWN][from];
				Zobrist::key ^= Zobrist::hash[capture][to];
				Zobrist::key ^= Zobrist::black_to_move;
				bitboards[B_QUEEN] ^= 1ull << to;
				bitboards[B_PAWN] ^= 1ull << from;
				bitboards[capture] ^= 1ull << to;
				black_pieces ^= 1ull << to;
				black_pieces ^= 1ull << from;
				white_pieces ^= 1ull << to;
				pieces[to] = capture;
				pieces[from] = B_PAWN;
				return;
			}
		}
		else {
			switch (movetype) {
			case NORMAL:
				Zobrist::key ^= Zobrist::hash[pieces[to]][to];
				Zobrist::key ^= Zobrist::hash[pieces[to]][from];
				Zobrist::key ^= Zobrist::hash[capture][to];
				Zobrist::key ^= Zobrist::black_to_move;
				bitboards[pieces[to]] ^= 1ull << to;
				bitboards[pieces[to]] ^= 1ull << from;
				bitboards[capture] ^= 1ull << to;
				black_pieces ^= 1ull << to;
				black_pieces ^= 1ull << from;
				white_pieces ^= square_bb(to) * non_null[capture];
				pieces[from] = pieces[to];
				pieces[to] = capture;
				return;
			case PROMOTION:
				Zobrist::key ^= Zobrist::hash[B_QUEEN][to];
				Zobrist::key ^= Zobrist::hash[B_PAWN][from];
				Zobrist::key ^= Zobrist::hash[capture][to];
				Zobrist::key ^= Zobrist::black_to_move;
				bitboards[B_QUEEN] ^= 1ull << to;
				bitboards[B_PAWN] ^= 1ull << from;
				bitboards[capture] ^= 1ull << to;
				black_pieces ^= 1ull << to;
				black_pieces ^= 1ull << from;
				white_pieces ^= square_bb(to) * non_null[capture];
				pieces[to] = capture;
				pieces[from] = B_PAWN;
				return;
			case SHORTCASTLE:
				bitboards[B_ROOK] ^= 0x500000000000000ull;
				bitboards[B_KING] ^= 0xa00000000000000ull;
				black_pieces ^= 0xf00000000000000ull;
				pieces[59] = B_KING;
				pieces[56] = B_ROOK;
				pieces[57] = NO_PIECE;
				pieces[58] = NO_PIECE;
				Zobrist::key ^= Zobrist::hash[B_KING][57];
				Zobrist::key ^= Zobrist::hash[B_ROOK][58];
				Zobrist::key ^= Zobrist::hash[B_KING][59];
				Zobrist::key ^= Zobrist::hash[B_ROOK][56];
				Zobrist::key ^= Zobrist::black_to_move;
				return;
			case LONGCASTLE:
				bitboards[B_ROOK] ^= 0x9000000000000000ull;
				bitboards[B_KING] ^= 0x2800000000000000ull;
				black_pieces ^= 0xb800000000000000ull;
				pieces[59] = B_KING;
				pieces[63] = B_ROOK;
				pieces[61] = NO_PIECE;
				pieces[60] = NO_PIECE;
				Zobrist::key ^= Zobrist::hash[B_KING][61];
				Zobrist::key ^= Zobrist::hash[B_ROOK][60];
				Zobrist::key ^= Zobrist::hash[B_KING][59];
				Zobrist::key ^= Zobrist::hash[B_ROOK][63];
				Zobrist::key ^= Zobrist::black_to_move;
				return;
			case ENPASSANT:
				Zobrist::key ^= Zobrist::hash[B_PAWN][to];
				Zobrist::key ^= Zobrist::hash[B_PAWN][from];
				Zobrist::key ^= Zobrist::hash[W_PAWN][to + 8];
				Zobrist::key ^= Zobrist::black_to_move;
				bitboards[B_PAWN] ^= 1ull << to;
				bitboards[B_PAWN] ^= 1ull << from;
				bitboards[W_PAWN] ^= 1ull << (to + 8);
				black_pieces ^= 1ull << to;
				black_pieces ^= 1ull << from;
				white_pieces |= bitboards[W_PAWN];
				pieces[to] = NO_PIECE;
				pieces[from] = B_PAWN;
				pieces[to + 8] = W_PAWN;
				return;
			}
		}
	}
}

}

#endif