
#include "perft.h"
#include "whitemovegenerator.h"
#include "blackmovegenerator.h"
#include "movegenerator.h"
#include "board.h"
#include "debug.h"
#include "ui.h"
#include "util.h"
#include "gamestate.h"
#include <thread>
#include <chrono>

namespace Chess {

namespace Perft {

	void go(int depth) {

		int plies_to_search = 1;

		if (GameState::white_to_move) {
			for (;plies_to_search <= depth; plies_to_search++) {
				leafnodes = 0;
				auto start = Util::curr_time_millis();
				int moves[90];
				MoveGenerator::generate_moves(true, moves);
				for (int i = 0; i < moves[END]; i++) {
					int capture = Board::piece_types[to_sq(moves[i])];
					uint8_t c_rights = GameState::castling_rights;
					Board::makemove<true, false>(moves[i]);
					expand<false>(plies_to_search - 1);
					Board::undomove<true, false>(moves[i], capture);
					GameState::castling_rights = c_rights;
				}
				auto delta = Util::curr_time_millis() - start;
				std::cout << "\nDepth " << plies_to_search << ": " << leafnodes << " (" << delta << " ms)\n";
			}
		}
		else {
			for (;plies_to_search <= depth; plies_to_search++) {
				leafnodes = 0;
				auto start = Util::curr_time_millis();
				BlackMoveGenerator g;
				g.generate_moves();
				for (int i = 0; i < g.move_count; i++) {
					int capture = Board::piece_types[(g.moves[i] >> 6) & lsb_6];
					uint8_t c_rights = GameState::castling_rights;
					Board::makemove<false, false>(g.moves[i]);
					expand<true>(plies_to_search - 1);
					Board::undomove<false, false>(g.moves[i], capture);
					GameState::castling_rights = c_rights;
				}
				auto delta = Util::curr_time_millis() - start;
				std::cout << "\nDepth " << plies_to_search << ": " << leafnodes << " (" << delta << " ms)\n";
			}
		}

	}

	template<bool white>
	void expand(int depth) {

		if (depth == 0) {
			leafnodes++;
			return;
		}

		if constexpr (white) {
			if (depth == 1) {
				leafnodes += MoveGenerator::bulk();
				return;
			}
			int moves[90];
			MoveGenerator::generate_moves(false, moves);
			for (int i = 0; i < moves[END]; i++) {
				int capture = Board::piece_types[to_sq(moves[i])];
				uint8_t c_rights = GameState::castling_rights;
				Board::makemove<true, false>(moves[i]);
				expand<false>(depth - 1);
				Board::undomove<true, false>(moves[i], capture);
				GameState::castling_rights = c_rights;
			}
		}
		else {
			if (depth == 1) {
				leafnodes += bulk_black();
				return;
			}
			BlackMoveGenerator g(false);
			g.generate_moves();
			for (int i = 0; i < g.move_count; i++) {
				int capture = Board::piece_types[(g.moves[i] >> 6) & lsb_6];
				uint8_t c_rights = GameState::castling_rights;
				Board::makemove<false, false>(g.moves[i]);
				expand<true>(depth - 1);
				Board::undomove<false, false>(g.moves[i], capture);
				GameState::castling_rights = c_rights;
			}
		}

	}

	int bulk_white() {

		WhiteMoveGenerator g(false);

		int count = 0;
		if (g.in_double_check) {
			int kingsq = tzcnt(Board::bitboards[WHITE_KING]);
			uint64_t move_map = Lookup::king_masks[kingsq] & ~(Board::white_pieces | g.seen_by_enemy);
			return popcnt(move_map);
		}
		uint64_t piece_map = Board::bitboards[WHITE_PAWN] & not_7_or_h & ~(g.pin_O | g.pinBD);
		uint64_t move_map = (piece_map << 7) & Board::black_pieces & g.checkmask;
		count += popcnt(move_map);
		piece_map = Board::bitboards[WHITE_PAWN] & not_7_or_a & ~(g.pin_O | g.pinFD);
		move_map = (piece_map << 9) & Board::black_pieces & g.checkmask;
		count += popcnt(move_map);
		piece_map = Board::bitboards[WHITE_PAWN] & ~seventh_rank & ~(g.pin_D | g.pinH);
		move_map = (piece_map << 8) & g.empty & g.checkmask;
		count += popcnt(move_map);
		move_map = (piece_map << 8) & g.empty;
		move_map = (move_map << 8) & fourth_rank & g.empty & g.checkmask;
		count += popcnt(move_map);
		piece_map = Board::bitboards[WHITE_KNIGHT] & g.not_pinned;
		int piece_square;
		for (; piece_map; piece_map = blsr(piece_map)) {
			piece_square = tzcnt(piece_map);
			move_map = Lookup::knight_masks[piece_square] & g.legal_squares;
			count += popcnt(move_map);
		}
		piece_map = g.BishopQueen & g.not_pinned;
		for (; piece_map; piece_map = blsr(piece_map)) {
			piece_square = tzcnt(piece_map);
			move_map = BISHOP_ATTACKS(piece_square, g.occupied) & g.legal_squares;
			count += popcnt(move_map);
		}
		piece_map = g.BishopQueen & g.pinFD;
		for (; piece_map; piece_map = blsr(piece_map)) {
			piece_square = tzcnt(piece_map);
			move_map = BISHOP_ATTACKS(piece_square, g.occupied) & g.legal_squares & g.pinFD;
			count += popcnt(move_map);
		}
		piece_map = g.BishopQueen & g.pinBD;
		for (; piece_map; piece_map = blsr(piece_map)) {
			piece_square = tzcnt(piece_map);
			move_map = BISHOP_ATTACKS(piece_square, g.occupied) & g.legal_squares & g.pinBD;
			count += popcnt(move_map);
		}
		piece_map = g.RookQueen & g.not_pinned;
		for (; piece_map; piece_map = blsr(piece_map)) {
			piece_square = tzcnt(piece_map);
			move_map = ROOK_ATTACKS(piece_square, g.occupied) & g.legal_squares;
			count += popcnt(move_map);
		}
		piece_map = g.RookQueen & g.pinV;
		for (; piece_map; piece_map = blsr(piece_map)) {
			piece_square = tzcnt(piece_map);
			move_map = ROOK_ATTACKS(piece_square, g.occupied) & g.legal_squares & g.pinV;
			count += popcnt(move_map);
		}
		piece_map = g.RookQueen & g.pinH;
		for (; piece_map; piece_map = blsr(piece_map)) {
			piece_square = tzcnt(piece_map);
			move_map = ROOK_ATTACKS(piece_square, g.occupied) & g.legal_squares & g.pinH;
			count += popcnt(move_map);
		}
		piece_square = tzcnt(Board::bitboards[WHITE_KING]);
		move_map = Lookup::king_masks[piece_square] & ~(Board::white_pieces | g.seen_by_enemy);
		count += popcnt(move_map);
		if (Board::bitboards[WHITE_PAWN] & seventh_rank) {
			uint64_t promotable = Board::bitboards[WHITE_PAWN] & seventh_rank;
			piece_map = promotable & ~(hfile | g.pin_O | g.pinBD);
			move_map = (piece_map << 7) & Board::black_pieces & g.checkmask;
			count += popcnt(move_map) * 4;
			piece_map = promotable & ~(afile | g.pin_O | g.pinFD);
			move_map = (piece_map << 9) & Board::black_pieces & g.checkmask;
			count += popcnt(move_map) * 4;
			piece_map = promotable & g.not_pinned;
			move_map = (piece_map << 8) & g.empty & g.checkmask;
			count += popcnt(move_map) * 4;
		}
		if (g.incheck()) return count;
		uint64_t legality = ((g.occupied | g.seen_by_enemy) & 0b110ull) | GameState::rights_K();
		count += legality == 8;
		legality = (g.occupied & 0x70ull) | (g.seen_by_enemy & 0x30ull) | GameState::rights_Q();
		count += legality == 4;

		return count;

	}

	int bulk_black() {

		BlackMoveGenerator g(false);

		int count = 0;
		if (g.in_double_check) {
			int kingsq = tzcnt(Board::bitboards[BLACK_KING]);
			uint64_t move_map = Lookup::king_masks[kingsq] & ~(Board::black_pieces | g.seen_by_enemy);
			return popcnt(move_map);
		}
		uint64_t piece_map = Board::bitboards[BLACK_PAWN] & not_2_or_a & ~(g.pin_O | g.pinBD);
		uint64_t move_map = (piece_map >> 7) & Board::white_pieces & g.checkmask;
		count += popcnt(move_map);
		piece_map = Board::bitboards[BLACK_PAWN] & not_2_or_h & ~(g.pin_O | g.pinFD);
		move_map = (piece_map >> 9) & Board::white_pieces & g.checkmask;
		count += popcnt(move_map);
		piece_map = Board::bitboards[BLACK_PAWN] & ~(g.pin_D | g.pinH | second_rank);
		move_map = (piece_map >> 8) & g.empty & g.checkmask;
		count += popcnt(move_map);
		move_map = (piece_map >> 8) & g.empty;
		move_map = (move_map >> 8) & g.empty & fifth_rank & g.checkmask;
		count += popcnt(move_map);
		piece_map = Board::bitboards[BLACK_KNIGHT] & g.not_pinned;
		int piece_square;
		for (; piece_map; piece_map = blsr(piece_map)) {
			piece_square = tzcnt(piece_map);
			move_map = Lookup::knight_masks[piece_square] & g.legal_squares;
			count += popcnt(move_map);
		}
		piece_map = g.BishopQueen & g.not_pinned;
		for (; piece_map; piece_map = blsr(piece_map)) {
			piece_square = tzcnt(piece_map);
			move_map = BISHOP_ATTACKS(piece_square, g.occupied) & g.legal_squares;
			count += popcnt(move_map);
		}
		piece_map = g.BishopQueen & g.pinFD;
		for (; piece_map; piece_map = blsr(piece_map)) {
			piece_square = tzcnt(piece_map);
			move_map = BISHOP_ATTACKS(piece_square, g.occupied) & g.legal_squares & g.pinFD;
			count += popcnt(move_map);
		}
		piece_map = g.BishopQueen & g.pinBD;
		for (; piece_map; piece_map = blsr(piece_map)) {
			piece_square = tzcnt(piece_map);
			move_map = BISHOP_ATTACKS(piece_square, g.occupied) & g.legal_squares & g.pinBD;
			count += popcnt(move_map);
		}
		piece_map = g.RookQueen & g.not_pinned;
		for (; piece_map; piece_map = blsr(piece_map)) {
			piece_square = tzcnt(piece_map);
			move_map = ROOK_ATTACKS(piece_square, g.occupied) & g.legal_squares;
			count += popcnt(move_map);
		}
		piece_map = g.RookQueen & g.pinV;
		for (; piece_map; piece_map = blsr(piece_map)) {
			piece_square = tzcnt(piece_map);
			move_map = ROOK_ATTACKS(piece_square, g.occupied) & g.legal_squares & g.pinV;
			count += popcnt(move_map);
		}
		piece_map = g.RookQueen & g.pinH;
		for (; piece_map; piece_map = blsr(piece_map)) {
			piece_square = tzcnt(piece_map);
			move_map = ROOK_ATTACKS(piece_square, g.occupied) & g.legal_squares & g.pinH;
			count += popcnt(move_map);
		}
		piece_square = tzcnt(Board::bitboards[BLACK_KING]);
		move_map = Lookup::king_masks[piece_square] & ~(Board::black_pieces | g.seen_by_enemy);
		count += popcnt(move_map);
		if (Board::bitboards[BLACK_PAWN] & second_rank) {
			uint64_t promotable = Board::bitboards[BLACK_PAWN] & second_rank;
			piece_map = promotable & ~(afile | g.pin_O | g.pinBD);
			move_map = (piece_map >> 7) & Board::white_pieces & g.checkmask;
			count += popcnt(move_map) * 4;
			piece_map = promotable & ~(hfile | g.pin_O | g.pinFD);
			move_map = (piece_map >> 9) & Board::white_pieces & g.checkmask;
			count += popcnt(move_map) * 4;
			piece_map = promotable & g.not_pinned;
			move_map = (piece_map >> 8) & g.empty & g.checkmask;
			count += popcnt(move_map) * 4;
		}
		if (g.incheck()) return count;
		uint64_t legality = ((g.occupied | g.seen_by_enemy) & 0x600000000000000ull) | GameState::rights_k();
		count += legality == 2;
		legality = (g.occupied & 0x7000000000000000ull) | (g.seen_by_enemy & 0x3000000000000000ull) | GameState::rights_q();
		count += legality == 1;
		
		return count;

	}
	
}

}
