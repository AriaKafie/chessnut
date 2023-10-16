
#include "blackmovegenerator.h"
#include "board.h"
#include "ui.h"
#include "defs.h"
#include "lookup.h"
#include "gamestate.h"
#include <intrin.h>
#include <iostream>

namespace Chess {

	BlackMoveGenerator::BlackMoveGenerator(bool enable_ep) : 
		ep_enabled(enable_ep), move_count(0), pinV(0), pinFD(0), pinH(0), pinBD(0), checkmask(0) {

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
		if (ep_enabled) gen_ep();
		if (Board::bitboards[BLACK_PAWN] & second_rank) gen_promotions();

	}

	void BlackMoveGenerator::init() {

		friendly_ksq = tzcnt(Board::bitboards[BLACK_KING]);
		checkmask |= Lookup::knight_masks[friendly_ksq] & Board::bitboards[WHITE_KNIGHT];
		checkmask |= Lookup::white_pawn_checkers[friendly_ksq] & Board::bitboards[WHITE_PAWN];
		BishopQueen = Board::bitboards[BLACK_BISHOP] | Board::bitboards[BLACK_QUEEN];
		RookQueen = Board::bitboards[BLACK_ROOK] | Board::bitboards[BLACK_QUEEN];
		occupied = Board::black_pieces | Board::white_pieces;
		empty = ~occupied;
		seen_by_enemy = squares_seen_by_white();
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

		uint64_t pawns = Board::bitboards[BLACK_PAWN] & not_2_or_a & (pinFD | not_pinned);
		for (uint64_t downleft = (pawns >> 7) & Board::white_pieces & checkmask; downleft; downleft = blsr(downleft)) {
			int to = tzcnt(downleft);
			moves[move_count++] = to + 7 + (to << 6);
		}
		pawns = Board::bitboards[BLACK_PAWN] & not_2_or_h & (pinBD | not_pinned);
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
		pawns = Board::bitboards[BLACK_PAWN] & (pinV | not_pinned);
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
		for (uint64_t from_map = BishopQueen & pin_D; from_map; from_map = blsr(from_map)) {
			int from = tzcnt(from_map);
			for (uint64_t to = BISHOP_ATTACKS(from, occupied) & legal_squares & Lookup::pinmask[friendly_ksq][from]; to; to = blsr(to))
				moves[move_count++] = from + (tzcnt(to) << 6);
		}
		for (uint64_t from_map = RookQueen & not_pinned; from_map; from_map = blsr(from_map)) {
			int from = tzcnt(from_map);
			for (uint64_t to = ROOK_ATTACKS(from, occupied) & legal_squares; to; to = blsr(to))
				moves[move_count++] = from + (tzcnt(to) << 6);
		}
		for (uint64_t from_map = RookQueen & pin_O; from_map; from_map = blsr(from_map)) {
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

		#define bb Board::bitboards

		if (uint64_t to_map = (bb[BLACK_PAWN] >> 7) & GameState::current_ep_square() & third_rank) {
			int to = tzcnt(to_map);
			moves[move_count++] = to + 7 + (to << 6) + ep_flag;
			uint64_t ep_toggle = to_map | (to_map << 7) | (to_map << 8);
			uint64_t occ = occupied ^ ep_toggle;
			uint64_t slider_checks =
				(BISHOP_ATTACKS(friendly_ksq, occ) & (bb[WHITE_QUEEN] | bb[WHITE_BISHOP]))
				| (ROOK_ATTACKS(friendly_ksq, occ) & (bb[WHITE_QUEEN] | bb[WHITE_ROOK]));
			if (slider_checks) move_count--;
		}
		if (uint64_t to_map = (bb[BLACK_PAWN] >> 9) & GameState::current_ep_square() & third_rank) {
			int to = tzcnt(to_map);
			moves[move_count++] = to + 9 + (to << 6) + ep_flag;
			uint64_t ep_toggle = to_map | (to_map << 9) | (to_map << 8);
			uint64_t occ = occupied ^ ep_toggle;
			uint64_t slider_checks =
				(BISHOP_ATTACKS(friendly_ksq, occ) & (bb[WHITE_QUEEN] | bb[WHITE_BISHOP]))
				| (ROOK_ATTACKS(friendly_ksq, occ) & (bb[WHITE_QUEEN] | bb[WHITE_ROOK]));
			if (slider_checks) move_count--;
		}

	}

	void BlackMoveGenerator::gen_castles() {

		if (incheck()) return;
		moves[move_count] = scastle_flag;
		uint64_t legality = ((occupied | seen_by_enemy) & 0x600000000000000ull) | GameState::rights_k();
		move_count += legality == 2;
		moves[move_count] = lcastle_flag;
		legality = (occupied & 0x7000000000000000ull) | (seen_by_enemy & 0x3000000000000000ull) | GameState::rights_q();
		move_count += legality == 1;

	}

	void BlackMoveGenerator::generate_BK() {

		for (uint64_t to = Lookup::king_masks[friendly_ksq] & ~(Board::black_pieces | seen_by_enemy); to; to = blsr(to))
			moves[move_count++] = friendly_ksq + (tzcnt(to) << 6);

	}

	uint64_t BlackMoveGenerator::squares_seen_by_white() {

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
