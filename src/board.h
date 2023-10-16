
#ifndef BOARD_H
#define BOARD_H

#include <string>
#include <cstdint>
#include "defs.h"
#include "gamestate.h"
#include "zobrist.h"

namespace Chess {

namespace Board {

	inline int piece_types[64];
	inline uint64_t bitboards[13];
	inline uint64_t white_pieces;
	inline uint64_t black_pieces;
	inline constexpr int non_null[13] = { 1,1,1,1,1,1,1,1,1,1,1,1,0 };

	void make_legal(bool white, int move);
	void undo_legal(bool white, int move, int capture);
	
	template<bool white, bool captures_only>
	inline void makemove(int move) {

		int from = from_sq(move);
		int to = to_sq(move);
		int movetype = type_of(move);

		if constexpr (white) {
			if constexpr (captures_only) {
				switch (movetype) {
				case STANDARD:
					Zobrist::key ^= Zobrist::hash[piece_types[from]][from];
					Zobrist::key ^= Zobrist::hash[piece_types[to]][to];
					Zobrist::key ^= Zobrist::hash[piece_types[from]][to];
					Zobrist::key ^= Zobrist::black_to_move;
					bitboards[piece_types[to]] ^= 1ull << to;
					bitboards[piece_types[from]] ^= 1ull << from;
					bitboards[piece_types[from]] ^= 1ull << to;
					white_pieces ^= 1ull << from;
					white_pieces ^= 1ull << to;
					black_pieces ^= 1ull << to;
					piece_types[to] = piece_types[from];
					piece_types[from] = NULLPIECE;
					GameState::castling_rights &= ((bitboards[WHITE_ROOK] & H1) << 3) | Qkq;
					GameState::castling_rights &= ((bitboards[WHITE_ROOK] & A1) >> 5) | Kkq;
					GameState::castling_rights &= ((bitboards[BLACK_ROOK] & H8) >> 55) | KQq;
					GameState::castling_rights &= ((bitboards[BLACK_ROOK] & A8) >> 63) | KQk;
					GameState::castling_rights &= (bitboards[WHITE_KING] & E1) | ((bitboards[WHITE_KING] & E1) >> 1) | kq;
					return;
				case PROMOTION:
					Zobrist::key ^= Zobrist::hash[WHITE_PAWN][from];
					Zobrist::key ^= Zobrist::hash[piece_types[to]][to];
					Zobrist::key ^= Zobrist::hash[WHITE_QUEEN][to];
					Zobrist::key ^= Zobrist::black_to_move;
					bitboards[piece_types[to]] ^= 1ull << to;
					bitboards[WHITE_PAWN] ^= 1ull << from;
					bitboards[WHITE_QUEEN] ^= 1ull << to;
					white_pieces ^= 1ull << from;
					white_pieces ^= 1ull << to;
					black_pieces ^= 1ull << to;
					piece_types[to] = WHITE_QUEEN;
					piece_types[from] = NULLPIECE;
					GameState::castling_rights &= ((bitboards[BLACK_ROOK] & H8) >> 55) | KQq;
					GameState::castling_rights &= ((bitboards[BLACK_ROOK] & A8) >> 63) | KQk;
					return;
				}
			}
			else {
				switch (movetype) {
				case STANDARD:
					Zobrist::key ^= Zobrist::hash[piece_types[from]][from];
					Zobrist::key ^= Zobrist::hash[piece_types[to]][to];
					Zobrist::key ^= Zobrist::hash[piece_types[from]][to];
					Zobrist::key ^= Zobrist::black_to_move;
					bitboards[piece_types[to]] ^= 1ull << to;
					bitboards[piece_types[from]] ^= 1ull << from;
					bitboards[piece_types[from]] ^= 1ull << to;
					white_pieces ^= 1ull << from;
					white_pieces ^= 1ull << to;
					black_pieces &= ~(1ull << to);
					piece_types[to] = piece_types[from];
					piece_types[from] = NULLPIECE;
					GameState::castling_rights &= ((bitboards[WHITE_ROOK] & H1) << 3) | Qkq;
					GameState::castling_rights &= ((bitboards[WHITE_ROOK] & A1) >> 5) | Kkq;
					GameState::castling_rights &= ((bitboards[BLACK_ROOK] & H8) >> 55) | KQq;
					GameState::castling_rights &= ((bitboards[BLACK_ROOK] & A8) >> 63) | KQk;
					GameState::castling_rights &= (bitboards[WHITE_KING] & E1) | ((bitboards[WHITE_KING] & E1) >> 1) | kq;
					return;
				case PROMOTION:
					Zobrist::key ^= Zobrist::hash[WHITE_PAWN][from];
					Zobrist::key ^= Zobrist::hash[piece_types[to]][to];
					Zobrist::key ^= Zobrist::hash[WHITE_QUEEN][to];
					Zobrist::key ^= Zobrist::black_to_move;
					bitboards[piece_types[to]] ^= 1ull << to;
					bitboards[WHITE_PAWN] ^= 1ull << from;
					bitboards[WHITE_QUEEN] ^= 1ull << to;
					white_pieces ^= 1ull << from;
					white_pieces ^= 1ull << to;
					black_pieces &= ~(1ull << to);
					piece_types[to] = WHITE_QUEEN;
					piece_types[from] = NULLPIECE;
					GameState::castling_rights &= ((bitboards[BLACK_ROOK] & H8) >> 55) | KQq;
					GameState::castling_rights &= ((bitboards[BLACK_ROOK] & A8) >> 63) | KQk;
					return;
				case SHORTCASTLE:
					bitboards[WHITE_ROOK] ^= 0x5ull;
					bitboards[WHITE_KING] ^= 0xaull;
					white_pieces ^= 0xfull;
					piece_types[3] = NULLPIECE;
					piece_types[0] = NULLPIECE;
					piece_types[1] = WHITE_KING;
					piece_types[2] = WHITE_ROOK;
					Zobrist::key ^= Zobrist::hash[WHITE_KING][3];
					Zobrist::key ^= Zobrist::hash[WHITE_ROOK][0];
					Zobrist::key ^= Zobrist::hash[WHITE_KING][1];
					Zobrist::key ^= Zobrist::hash[WHITE_ROOK][2];
					Zobrist::key ^= Zobrist::black_to_move;
					GameState::castling_rights &= kq;
					return;
				case LONGCASTLE:
					bitboards[WHITE_ROOK] ^= 0x90ull;
					bitboards[WHITE_KING] ^= 0x28ull;
					white_pieces ^= 0xb8ull;
					piece_types[3] = NULLPIECE;
					piece_types[7] = NULLPIECE;
					piece_types[5] = WHITE_KING;
					piece_types[4] = WHITE_ROOK;
					Zobrist::key ^= Zobrist::hash[WHITE_KING][3];
					Zobrist::key ^= Zobrist::hash[WHITE_ROOK][7];
					Zobrist::key ^= Zobrist::hash[WHITE_KING][5];
					Zobrist::key ^= Zobrist::hash[WHITE_ROOK][4];
					Zobrist::key ^= Zobrist::black_to_move;
					GameState::castling_rights &= kq;
					return;
				case ENPASSANT:
					Zobrist::key ^= Zobrist::hash[WHITE_PAWN][from];
					Zobrist::key ^= Zobrist::hash[BLACK_PAWN][to - 8];
					Zobrist::key ^= Zobrist::hash[WHITE_PAWN][to];
					Zobrist::key ^= Zobrist::black_to_move;
					bitboards[WHITE_PAWN] ^= 1ull << from;
					bitboards[WHITE_PAWN] ^= 1ull << to;
					bitboards[BLACK_PAWN] ^= 1ull << (to - 8);
					white_pieces ^= 1ull << from;
					white_pieces ^= 1ull << to;
					black_pieces ^= 1ull << (to - 8);
					piece_types[from] = NULLPIECE;
					piece_types[to] = WHITE_PAWN;
					piece_types[to - 8] = NULLPIECE;
					return;
				}
			}
		}
		else {
			if constexpr (captures_only) {
				switch (movetype) {
				case STANDARD:
					Zobrist::key ^= Zobrist::black_to_move;
					Zobrist::key ^= Zobrist::hash[piece_types[from]][from];
					Zobrist::key ^= Zobrist::hash[piece_types[to]][to];
					Zobrist::key ^= Zobrist::hash[piece_types[from]][to];
					bitboards[piece_types[to]] ^= 1ull << to;
					bitboards[piece_types[from]] ^= 1ull << from;
					bitboards[piece_types[from]] ^= 1ull << to;
					black_pieces ^= 1ull << from;
					black_pieces ^= 1ull << to;
					white_pieces ^= 1ull << to;
					piece_types[to] = piece_types[from];
					piece_types[from] = NULLPIECE;
					GameState::castling_rights &= ((bitboards[WHITE_ROOK] & H1) << 3) | Qkq;
					GameState::castling_rights &= ((bitboards[WHITE_ROOK] & A1) >> 5) | Kkq;
					GameState::castling_rights &= ((bitboards[BLACK_ROOK] & H8) >> 55) | KQq;
					GameState::castling_rights &= ((bitboards[BLACK_ROOK] & A8) >> 63) | KQk;
					GameState::castling_rights &= ((bitboards[BLACK_KING] & E8) >> 58) | ((bitboards[BLACK_KING] & E8) >> 59) | KQ;
					return;
				case PROMOTION:
					Zobrist::key ^= Zobrist::black_to_move;
					Zobrist::key ^= Zobrist::hash[BLACK_PAWN][from];
					Zobrist::key ^= Zobrist::hash[piece_types[to]][to];
					Zobrist::key ^= Zobrist::hash[BLACK_QUEEN][to];
					bitboards[piece_types[to]] ^= 1ull << to;
					bitboards[BLACK_PAWN] ^= 1ull << from;
					bitboards[BLACK_QUEEN] ^= 1ull << to;
					black_pieces ^= 1ull << from;
					black_pieces ^= 1ull << to;
					white_pieces ^= 1ull << to;
					piece_types[to] = BLACK_QUEEN;
					piece_types[from] = NULLPIECE;
					GameState::castling_rights &= ((bitboards[WHITE_ROOK] & H1) << 3) | Qkq;
					GameState::castling_rights &= ((bitboards[WHITE_ROOK] & A1) >> 5) | Kkq;
					return;
				}
			}
			else {
				switch (movetype) {
				case STANDARD:
					Zobrist::key ^= Zobrist::black_to_move;
					Zobrist::key ^= Zobrist::hash[piece_types[from]][from];
					Zobrist::key ^= Zobrist::hash[piece_types[to]][to];
					Zobrist::key ^= Zobrist::hash[piece_types[from]][to];
					bitboards[piece_types[to]] ^= 1ull << to;
					bitboards[piece_types[from]] ^= 1ull << from;
					bitboards[piece_types[from]] ^= 1ull << to;
					black_pieces ^= 1ull << from;
					black_pieces ^= 1ull << to;
					white_pieces &= ~(1ull << to);
					piece_types[to] = piece_types[from];
					piece_types[from] = NULLPIECE;
					GameState::castling_rights &= ((bitboards[WHITE_ROOK] & H1) << 3) | Qkq;
					GameState::castling_rights &= ((bitboards[WHITE_ROOK] & A1) >> 5) | Kkq;
					GameState::castling_rights &= ((bitboards[BLACK_ROOK] & H8) >> 55) | KQq;
					GameState::castling_rights &= ((bitboards[BLACK_ROOK] & A8) >> 63) | KQk;
					GameState::castling_rights &= ((bitboards[BLACK_KING] & E8) >> 58) | ((bitboards[BLACK_KING] & E8) >> 59) | KQ;
					return;
				case PROMOTION:
					Zobrist::key ^= Zobrist::black_to_move;
					Zobrist::key ^= Zobrist::hash[BLACK_PAWN][from];
					Zobrist::key ^= Zobrist::hash[piece_types[to]][to];
					Zobrist::key ^= Zobrist::hash[BLACK_QUEEN][to];
					bitboards[piece_types[to]] ^= 1ull << to;
					bitboards[BLACK_PAWN] ^= 1ull << from;
					bitboards[BLACK_QUEEN] ^= 1ull << to;
					black_pieces ^= 1ull << from;
					black_pieces ^= 1ull << to;
					white_pieces &= ~(1ull << to);
					piece_types[to] = BLACK_QUEEN;
					piece_types[from] = NULLPIECE;
					GameState::castling_rights &= ((bitboards[WHITE_ROOK] & H1) << 3) | Qkq;
					GameState::castling_rights &= ((bitboards[WHITE_ROOK] & A1) >> 5) | Kkq;
					return;
				case SHORTCASTLE:
					bitboards[BLACK_ROOK] ^= 0x500000000000000ull;
					bitboards[BLACK_KING] ^= 0xa00000000000000ull;
					black_pieces ^= 0xf00000000000000ull;
					piece_types[59] = NULLPIECE;
					piece_types[56] = NULLPIECE;
					piece_types[57] = BLACK_KING;
					piece_types[58] = BLACK_ROOK;
					Zobrist::key ^= Zobrist::black_to_move;
					Zobrist::key ^= Zobrist::hash[BLACK_KING][59];
					Zobrist::key ^= Zobrist::hash[BLACK_ROOK][56];
					Zobrist::key ^= Zobrist::hash[BLACK_KING][57];
					Zobrist::key ^= Zobrist::hash[BLACK_ROOK][58];
					GameState::castling_rights &= KQ;
					return;
				case LONGCASTLE:
					bitboards[BLACK_ROOK] ^= 0x9000000000000000ull;
					bitboards[BLACK_KING] ^= 0x2800000000000000ull;
					black_pieces ^= 0xb800000000000000ull;
					piece_types[59] = NULLPIECE;
					piece_types[63] = NULLPIECE;
					piece_types[61] = BLACK_KING;
					piece_types[60] = BLACK_ROOK;
					Zobrist::key ^= Zobrist::black_to_move;
					Zobrist::key ^= Zobrist::hash[BLACK_KING][59];
					Zobrist::key ^= Zobrist::hash[BLACK_ROOK][63];
					Zobrist::key ^= Zobrist::hash[BLACK_KING][61];
					Zobrist::key ^= Zobrist::hash[BLACK_ROOK][60];
					GameState::castling_rights &= KQ;
					return;
				case ENPASSANT:
					Zobrist::key ^= Zobrist::black_to_move;
					Zobrist::key ^= Zobrist::hash[BLACK_PAWN][from];
					Zobrist::key ^= Zobrist::hash[WHITE_PAWN][to + 8];
					Zobrist::key ^= Zobrist::hash[BLACK_PAWN][to];
					bitboards[BLACK_PAWN] ^= 1ull << from;
					bitboards[BLACK_PAWN] ^= 1ull << to;
					bitboards[WHITE_PAWN] ^= 1ull << (to + 8);
					black_pieces ^= 1ull << from;
					black_pieces ^= 1ull << to;
					white_pieces ^= 1ull << (to + 8);
					piece_types[from] = NULLPIECE;
					piece_types[to] = BLACK_PAWN;
					piece_types[to + 8] = NULLPIECE;
					return;
				}
			}
		}
	}

	template<bool white, bool captures_only>
	inline void undomove(int move, int capture_type) {

		int from = from_sq(move);
		int to = to_sq(move);
		int movetype = type_of(move);

		if constexpr (white) {
			if constexpr (captures_only) {
				switch (movetype) {
				case STANDARD:
					Zobrist::key ^= Zobrist::black_to_move;
					Zobrist::key ^= Zobrist::hash[piece_types[to]][to];
					Zobrist::key ^= Zobrist::hash[piece_types[to]][from];
					Zobrist::key ^= Zobrist::hash[capture_type][to];
					bitboards[piece_types[to]] ^= 1ull << to;
					bitboards[piece_types[to]] ^= 1ull << from;
					bitboards[capture_type] ^= 1ull << to;
					white_pieces ^= 1ull << to;
					white_pieces ^= 1ull << from;
					black_pieces ^= 1ull << to;
					piece_types[from] = piece_types[to];
					piece_types[to] = capture_type;
					return;
				case PROMOTION:
					Zobrist::key ^= Zobrist::black_to_move;
					Zobrist::key ^= Zobrist::hash[WHITE_QUEEN][to];
					Zobrist::key ^= Zobrist::hash[WHITE_PAWN][from];
					Zobrist::key ^= Zobrist::hash[capture_type][to];
					bitboards[WHITE_QUEEN] ^= 1ull << to;
					bitboards[WHITE_PAWN] ^= 1ull << from;
					bitboards[capture_type] ^= 1ull << to;
					white_pieces ^= 1ull << to;
					white_pieces ^= 1ull << from;
					black_pieces ^= 1ull << to;
					piece_types[to] = capture_type;
					piece_types[from] = WHITE_PAWN;
					return;
				}
			}
			else {
				switch (movetype) {
				case STANDARD:
					Zobrist::key ^= Zobrist::black_to_move;
					Zobrist::key ^= Zobrist::hash[piece_types[to]][to];
					Zobrist::key ^= Zobrist::hash[piece_types[to]][from];
					Zobrist::key ^= Zobrist::hash[capture_type][to];
					bitboards[piece_types[to]] ^= 1ull << to;
					bitboards[piece_types[to]] ^= 1ull << from;
					bitboards[capture_type] ^= 1ull << to;
					white_pieces ^= 1ull << to;
					white_pieces ^= 1ull << from;
					black_pieces ^= (1ull << to) * non_null[capture_type];
					piece_types[from] = piece_types[to];
					piece_types[to] = capture_type;
					return;
				case PROMOTION:
					Zobrist::key ^= Zobrist::black_to_move;
					Zobrist::key ^= Zobrist::hash[WHITE_QUEEN][to];
					Zobrist::key ^= Zobrist::hash[WHITE_PAWN][from];
					Zobrist::key ^= Zobrist::hash[capture_type][to];
					bitboards[WHITE_QUEEN] ^= 1ull << to;
					bitboards[WHITE_PAWN] ^= 1ull << from;
					bitboards[capture_type] ^= 1ull << to;
					white_pieces ^= 1ull << to;
					white_pieces ^= 1ull << from;
					black_pieces ^= (1ull << to) * non_null[capture_type];
					piece_types[to] = capture_type;
					piece_types[from] = WHITE_PAWN;
					return;
				case SHORTCASTLE:
					bitboards[WHITE_ROOK] ^= 0x5ull;
					bitboards[WHITE_KING] ^= 0xaull;
					white_pieces ^= 0xfull;
					piece_types[3] = WHITE_KING;
					piece_types[0] = WHITE_ROOK;
					piece_types[1] = NULLPIECE;
					piece_types[2] = NULLPIECE;
					Zobrist::key ^= Zobrist::black_to_move;
					Zobrist::key ^= Zobrist::hash[WHITE_KING][1];
					Zobrist::key ^= Zobrist::hash[WHITE_ROOK][2];
					Zobrist::key ^= Zobrist::hash[WHITE_KING][3];
					Zobrist::key ^= Zobrist::hash[WHITE_ROOK][0];
					return;
				case LONGCASTLE:
					bitboards[WHITE_ROOK] ^= 0x90ull;
					bitboards[WHITE_KING] ^= 0x28ull;
					white_pieces ^= 0xb8ull;
					piece_types[3] = WHITE_KING;
					piece_types[7] = WHITE_ROOK;
					piece_types[5] = NULLPIECE;
					piece_types[4] = NULLPIECE;
					Zobrist::key ^= Zobrist::black_to_move;
					Zobrist::key ^= Zobrist::hash[WHITE_KING][5];
					Zobrist::key ^= Zobrist::hash[WHITE_ROOK][4];
					Zobrist::key ^= Zobrist::hash[WHITE_KING][3];
					Zobrist::key ^= Zobrist::hash[WHITE_ROOK][7];
					return;
				case ENPASSANT:
					Zobrist::key ^= Zobrist::black_to_move;
					Zobrist::key ^= Zobrist::hash[WHITE_PAWN][to];
					Zobrist::key ^= Zobrist::hash[WHITE_PAWN][from];
					Zobrist::key ^= Zobrist::hash[BLACK_PAWN][to - 8];
					bitboards[WHITE_PAWN] ^= 1ull << to;
					bitboards[WHITE_PAWN] ^= 1ull << from;
					bitboards[BLACK_PAWN] ^= 1ull << (to - 8);
					white_pieces ^= 1ull << to;
					white_pieces ^= 1ull << from;
					black_pieces |= bitboards[BLACK_PAWN];
					piece_types[to] = NULLPIECE;
					piece_types[from] = WHITE_PAWN;
					piece_types[to - 8] = BLACK_PAWN;
					return;
				}
			}
		}
		else {
			if constexpr (captures_only) {
				switch (movetype) {
				case STANDARD:
					Zobrist::key ^= Zobrist::hash[piece_types[to]][to];
					Zobrist::key ^= Zobrist::hash[piece_types[to]][from];
					Zobrist::key ^= Zobrist::hash[capture_type][to];
					Zobrist::key ^= Zobrist::black_to_move;
					bitboards[piece_types[to]] ^= 1ull << to;
					bitboards[piece_types[to]] ^= 1ull << from;
					bitboards[capture_type] ^= 1ull << to;
					black_pieces ^= 1ull << to;
					black_pieces ^= 1ull << from;
					white_pieces ^= 1ull << to;
					piece_types[from] = piece_types[to];
					piece_types[to] = capture_type;
					return;
				case PROMOTION:
					Zobrist::key ^= Zobrist::hash[BLACK_QUEEN][to];
					Zobrist::key ^= Zobrist::hash[BLACK_PAWN][from];
					Zobrist::key ^= Zobrist::hash[capture_type][to];
					Zobrist::key ^= Zobrist::black_to_move;
					bitboards[BLACK_QUEEN] ^= 1ull << to;
					bitboards[BLACK_PAWN] ^= 1ull << from;
					bitboards[capture_type] ^= 1ull << to;
					black_pieces ^= 1ull << to;
					black_pieces ^= 1ull << from;
					white_pieces ^= 1ull << to;
					piece_types[to] = capture_type;
					piece_types[from] = BLACK_PAWN;
					return;
				}
			}
			else {
				switch (movetype) {
				case STANDARD:
					Zobrist::key ^= Zobrist::hash[piece_types[to]][to];
					Zobrist::key ^= Zobrist::hash[piece_types[to]][from];
					Zobrist::key ^= Zobrist::hash[capture_type][to];
					Zobrist::key ^= Zobrist::black_to_move;
					bitboards[piece_types[to]] ^= 1ull << to;
					bitboards[piece_types[to]] ^= 1ull << from;
					bitboards[capture_type] ^= 1ull << to;
					black_pieces ^= 1ull << to;
					black_pieces ^= 1ull << from;
					white_pieces ^= (1ull << to) * non_null[capture_type];
					piece_types[from] = piece_types[to];
					piece_types[to] = capture_type;
					return;
				case PROMOTION:
					Zobrist::key ^= Zobrist::hash[BLACK_QUEEN][to];
					Zobrist::key ^= Zobrist::hash[BLACK_PAWN][from];
					Zobrist::key ^= Zobrist::hash[capture_type][to];
					Zobrist::key ^= Zobrist::black_to_move;
					bitboards[BLACK_QUEEN] ^= 1ull << to;
					bitboards[BLACK_PAWN] ^= 1ull << from;
					bitboards[capture_type] ^= 1ull << to;
					black_pieces ^= 1ull << to;
					black_pieces ^= 1ull << from;
					white_pieces ^= (1ull << to) * non_null[capture_type];
					piece_types[to] = capture_type;
					piece_types[from] = BLACK_PAWN;
					return;
				case SHORTCASTLE:
					bitboards[BLACK_ROOK] ^= 0x500000000000000ull;
					bitboards[BLACK_KING] ^= 0xa00000000000000ull;
					black_pieces ^= 0xf00000000000000ull;
					piece_types[59] = BLACK_KING;
					piece_types[56] = BLACK_ROOK;
					piece_types[57] = NULLPIECE;
					piece_types[58] = NULLPIECE;
					Zobrist::key ^= Zobrist::hash[BLACK_KING][57];
					Zobrist::key ^= Zobrist::hash[BLACK_ROOK][58];
					Zobrist::key ^= Zobrist::hash[BLACK_KING][59];
					Zobrist::key ^= Zobrist::hash[BLACK_ROOK][56];
					Zobrist::key ^= Zobrist::black_to_move;
					return;
				case LONGCASTLE:
					bitboards[BLACK_ROOK] ^= 0x9000000000000000ull;
					bitboards[BLACK_KING] ^= 0x2800000000000000ull;
					black_pieces ^= 0xb800000000000000ull;
					piece_types[59] = BLACK_KING;
					piece_types[63] = BLACK_ROOK;
					piece_types[61] = NULLPIECE;
					piece_types[60] = NULLPIECE;
					Zobrist::key ^= Zobrist::hash[BLACK_KING][61];
					Zobrist::key ^= Zobrist::hash[BLACK_ROOK][60];
					Zobrist::key ^= Zobrist::hash[BLACK_KING][59];
					Zobrist::key ^= Zobrist::hash[BLACK_ROOK][63];
					Zobrist::key ^= Zobrist::black_to_move;
					return;
				case ENPASSANT:
					Zobrist::key ^= Zobrist::hash[BLACK_PAWN][to];
					Zobrist::key ^= Zobrist::hash[BLACK_PAWN][from];
					Zobrist::key ^= Zobrist::hash[WHITE_PAWN][to + 8];
					Zobrist::key ^= Zobrist::black_to_move;
					bitboards[BLACK_PAWN] ^= 1ull << to;
					bitboards[BLACK_PAWN] ^= 1ull << from;
					bitboards[WHITE_PAWN] ^= 1ull << (to + 8);
					black_pieces ^= 1ull << to;
					black_pieces ^= 1ull << from;
					white_pieces |= bitboards[WHITE_PAWN];
					piece_types[to] = NULLPIECE;
					piece_types[from] = BLACK_PAWN;
					piece_types[to + 8] = WHITE_PAWN;
					return;
				}
			}
		}
	}

}

}

#endif
