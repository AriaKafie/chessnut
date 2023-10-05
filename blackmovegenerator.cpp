
#include "BlackMoveGenerator.h"
#include "Board.h"
#include "UI.h"
#include "Defs.h"
#include "Lookup.h"
#include "GameState.h"
#include <intrin.h>
#include <iostream>

namespace Chess {

	BlackMoveGenerator::BlackMoveGenerator(bool generate_enpassant_) : generate_enpassant(generate_enpassant_) {

		move_count = 0;
		init();
		if (checkmask == 0) checkmask = MAX_LONG;
		legal_squares = checkmask & ~Board::black_pieces;

	}

	void BlackMoveGenerator::generate_moves() {

		if (in_double_check) {
			generate_BK();
			return;
		}
		generate();
		gen_castles();
		if (generate_enpassant) gen_ep();
		if (Board::bitboards[BLACK_PAWN] & second_rank) gen_promotions();

	}

	void BlackMoveGenerator::init() {

		pinV = 0;
		pinFD = 0;
		pinH = 0;
		pinBD = 0;
		checkmask = 0;
		friendly_ksq = tzcnt(Board::bitboards[BLACK_KING]);
		checkmask |= Lookup::knight_masks[friendly_ksq] & Board::bitboards[WHITE_KNIGHT];
		checkmask |= Lookup::white_pawn_checkers[friendly_ksq] & Board::bitboards[WHITE_PAWN];
		BishopQueen = Board::bitboards[BLACK_BISHOP] | Board::bitboards[BLACK_QUEEN];
		RookQueen = Board::bitboards[BLACK_ROOK] | Board::bitboards[BLACK_QUEEN];
		occupied = Board::black_pieces | Board::white_pieces;
		empty = ~occupied;
		seen_by_enemy = squares_controlled_by_white();
		uint64_t checkers = 
		(BISHOP_ATTACKS(friendly_ksq, Board::white_pieces) & (Board::bitboards[WHITE_QUEEN] | Board::bitboards[WHITE_BISHOP]))
		| (ROOK_ATTACKS(friendly_ksq, Board::white_pieces) & (Board::bitboards[WHITE_QUEEN] | Board::bitboards[WHITE_ROOK]));
	
		for (;checkers; checkers = blsr(checkers)) {
			uint64_t checkray = Lookup::checkmask[friendly_ksq][tzcnt(checkers)];
			uint64_t crossfire_blacks = Board::black_pieces & checkray;
			checkmask |= checkray * (crossfire_blacks == 0);
			uint64_t pinray = checkray * (popcnt(crossfire_blacks) == 1);
			pinV  |= pinray & Lookup::vertical[friendly_ksq];
			pinFD |= pinray & Lookup::forward_diagonal[friendly_ksq];
			pinH  |= pinray & Lookup::horizontal[friendly_ksq];
			pinBD |= pinray & Lookup::back_diagonal[friendly_ksq];
		}

		pin_O = pinV | pinH;
		pin_D = pinFD | pinBD;
		not_pinned = ~(pin_O | pin_D);
		in_double_check = (popcnt(Lookup::double_check[friendly_ksq] & checkmask) > 1);

	}

	void BlackMoveGenerator::generate() {

		uint64_t pawns = Board::bitboards[BLACK_PAWN] & not_2_or_a & ~(pin_O | pinBD);
		for (uint64_t downleft = (pawns >> 7) & Board::white_pieces & checkmask; downleft; downleft = blsr(downleft)) {
			int to = tzcnt(downleft);
			moves[move_count++] = to + 7 + (to << 6);
		}
		pawns = Board::bitboards[BLACK_PAWN] & not_2_or_h & ~(pin_O | pinFD);
		for (uint64_t downright = (pawns >> 9) & Board::white_pieces & checkmask; downright; downright = blsr(downright)) {
			int to = tzcnt(downright);
			moves[move_count++] = to + 9 + (to << 6);
		}
		pawns = Board::bitboards[BLACK_PAWN] & ~(pin_D | pinH | second_rank);
		for (uint64_t push = (pawns >> 8) & empty & checkmask; push; push = blsr(push)) {
			int to = tzcnt(push);
			moves[move_count++] = to + 8 + (to << 6);
		}
		uint64_t r = ((sixth_rank & empty) >> 8) & empty;
		pawns = Board::bitboards[BLACK_PAWN] & ~(pin_D | pinH);
		for (uint64_t double_push = (pawns >> 16) & r & checkmask; double_push; double_push = blsr(double_push)) {
			int to = tzcnt(double_push);
			moves[move_count++] = to + 16 + (to << 6);
		}
		for (uint64_t from_map = Board::bitboards[BLACK_KNIGHT] & not_pinned; from_map; from_map = blsr(from_map)) {
			int from = tzcnt(from_map);
			for (uint64_t to = Lookup::knight_masks[from] & legal_squares; to; to = blsr(to))
				moves[move_count++] = from + (tzcnt(to) << 6);
		}
		for (uint64_t from_map = BishopQueen & not_pinned; from_map; from_map = blsr(from_map)) {
			int from = tzcnt(from_map);
			for (uint64_t to = BISHOP_ATTACKS(from, occupied) & legal_squares; to; to = blsr(to))
				moves[move_count++] = from + (tzcnt(to) << 6);
		}
		for (uint64_t from_map = BishopQueen & ~not_pinned; from_map; from_map = blsr(from_map)) {
			int from = tzcnt(from_map);
			for (uint64_t to = BISHOP_ATTACKS(from, occupied) & legal_squares & Lookup::pinmask[friendly_ksq][from]; to; to = blsr(to))
				moves[move_count++] = from + (tzcnt(to) << 6);
		}
		for (uint64_t from_map = RookQueen & not_pinned; from_map; from_map = blsr(from_map)) {
			int from = tzcnt(from_map);
			for (uint64_t to = ROOK_ATTACKS(from, occupied) & legal_squares; to; to = blsr(to))
				moves[move_count++] = from + (tzcnt(to) << 6);
		}
		for (uint64_t from_map = RookQueen & ~not_pinned; from_map; from_map = blsr(from_map)) {
			int from = tzcnt(from_map);
			for (uint64_t to = ROOK_ATTACKS(from, occupied) & legal_squares & Lookup::pinmask[friendly_ksq][from]; to; to = blsr(to))
				moves[move_count++] = from + (tzcnt(to) << 6);
		}
		for (uint64_t to = Lookup::king_masks[friendly_ksq] & ~(Board::black_pieces | seen_by_enemy); to; to = blsr(to))
			moves[move_count++] = friendly_ksq + (tzcnt(to) << 6);

	}

	void BlackMoveGenerator::gen_promotions() {

		uint64_t pawns_on_2 = Board::bitboards[BLACK_PAWN] & second_rank;
		uint64_t pawns = pawns_on_2 & ~(afile | pin_O | pinBD);
		for (uint64_t downleft = (pawns >> 7) & Board::white_pieces & checkmask; downleft; downleft = blsr(downleft)) {
			int to = tzcnt(downleft);
			moves[move_count++] = to + 7 + (to << 6) + promotion_flag;
		}
		pawns = pawns_on_2 & ~(hfile | pin_O | pinFD);
		for (uint64_t downright = (pawns >> 9) & Board::white_pieces & checkmask; downright; downright = blsr(downright)) {
			int to = tzcnt(downright);
			moves[move_count++] = to + 9 + (to << 6) + promotion_flag;
		}
		pawns = pawns_on_2 & not_pinned;
		for (uint64_t push = (pawns >> 8) & empty & checkmask; push; push = blsr(push)) {
			int to = tzcnt(push);
			moves[move_count++] = to + 8 + (to << 6) + promotion_flag;
		}

	}

	void BlackMoveGenerator::gen_ep() {

		uint64_t move_map = (Board::bitboards[BLACK_PAWN] >> 7) & GameState::current_ep_square() & third_rank;
		if (move_map && !((Board::bitboards[BLACK_KING] & third_rank) && ((Board::bitboards[WHITE_QUEEN] | Board::bitboards[WHITE_ROOK]) & third_rank))) { // <- not perfect, but ensures king cant be captured
			int move_square = tzcnt(move_map);
			moves[move_count++] = move_square + 7 + (move_square << 6) + ep_flag;
		}
		move_map = (Board::bitboards[BLACK_PAWN] >> 9) & GameState::current_ep_square() & third_rank;
		if (move_map && !((Board::bitboards[BLACK_KING] & third_rank) && ((Board::bitboards[WHITE_QUEEN] | Board::bitboards[WHITE_ROOK]) & third_rank))) {
			int move_square = tzcnt(move_map);
			moves[move_count++] = move_square + 9 + (move_square << 6) + ep_flag;
		}

	}

	void BlackMoveGenerator::gen_castles() {

		if (~checkmask) return;
		if ((!(occupied & 0x600000000000000ull)) && !is_attacked(0x600000000000000ull) && GameState::rights_k())
			moves[move_count++] = scastle_flag;
		if ((!(occupied & 0x7000000000000000ull)) && !is_attacked(0x3000000000000000ull) && GameState::rights_q())
			moves[move_count++] = lcastle_flag;

	}

	void BlackMoveGenerator::generate_BK() {

		for (uint64_t to = Lookup::king_masks[friendly_ksq] & ~(Board::black_pieces | seen_by_enemy); to; to = blsr(to))
			moves[move_count++] = friendly_ksq + (tzcnt(to) << 6);

	}

	inline bool BlackMoveGenerator::is_attacked(uint64_t squares) {

		return squares & seen_by_enemy;

	}

	uint64_t BlackMoveGenerator::squares_controlled_by_white() {

		uint64_t occ_kingless = occupied ^ (1ull << friendly_ksq);
		uint64_t protected_squares = ((Board::bitboards[WHITE_PAWN] << 9) & NOT_FILE_H) | ((Board::bitboards[WHITE_PAWN] << 7) & NOT_FILE_A);

		for (uint64_t piece_map = Board::bitboards[WHITE_KNIGHT]; piece_map; piece_map = blsr(piece_map))
			protected_squares |= Lookup::knight_masks[tzcnt(piece_map)];
		for (uint64_t piece_map = Board::bitboards[WHITE_BISHOP] | Board::bitboards[WHITE_QUEEN]; piece_map; piece_map = blsr(piece_map))
			protected_squares |= BISHOP_ATTACKS(tzcnt(piece_map), occ_kingless);
		for (uint64_t piece_map = Board::bitboards[WHITE_ROOK] | Board::bitboards[WHITE_QUEEN]; piece_map; piece_map = blsr(piece_map))
			protected_squares |= ROOK_ATTACKS(tzcnt(piece_map), occ_kingless);
		return protected_squares | Lookup::king_masks[tzcnt(Board::bitboards[WHITE_KING])];

	}

}
