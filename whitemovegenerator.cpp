
#include "WhiteMoveGenerator.h"
#include "Board.h"
#include "UI.h"
#include "Lookup.h"
#include "Defs.h"
#include "GameState.h"
#include <intrin.h>
#include <iostream>

namespace Chess {

	WhiteMoveGenerator::WhiteMoveGenerator(bool generate_enpassant_) : generate_enpassant(generate_enpassant_) {

		move_count = 0;
		init();
		if (checkmask == 0) checkmask = MAX_LONG;
		legal_squares = checkmask & ~Board::white_pieces;

	}

	void WhiteMoveGenerator::generate_moves() {

		if (in_double_check) {
			generate_WK();
			return;
		}
		generate();
		gen_castles();
		if (generate_enpassant) gen_ep();
		if (Board::bitboards[WHITE_PAWN] & seventh_rank) gen_promotions();

	}

	void WhiteMoveGenerator::init() {

		pinV = 0;
		pinFD = 0;
		pinH = 0;
		pinBD = 0;
		checkmask = 0;
		friendly_ksq = tzcnt(Board::bitboards[WHITE_KING]);
		checkmask |= Lookup::knight_masks[friendly_ksq] & Board::bitboards[BLACK_KNIGHT];
		checkmask |= Lookup::black_pawn_checkers[friendly_ksq] & Board::bitboards[BLACK_PAWN];
		BishopQueen = Board::bitboards[WHITE_BISHOP] | Board::bitboards[WHITE_QUEEN];
		RookQueen = Board::bitboards[WHITE_ROOK] | Board::bitboards[WHITE_QUEEN];
		occupied = Board::black_pieces | Board::white_pieces;
		empty = ~occupied;
		seen_by_enemy = squares_controlled_by_black();
		uint64_t checkers =
			(BISHOP_ATTACKS(friendly_ksq, Board::black_pieces) & (Board::bitboards[BLACK_QUEEN] | Board::bitboards[BLACK_BISHOP]))
			| (ROOK_ATTACKS(friendly_ksq, Board::black_pieces) & (Board::bitboards[BLACK_QUEEN] | Board::bitboards[BLACK_ROOK]));

		for (; checkers; checkers = blsr(checkers)) {
			uint64_t checkray = Lookup::checkmask[friendly_ksq][tzcnt(checkers)];
			uint64_t crossfire_whites = Board::white_pieces & checkray;
			checkmask |= checkray * (crossfire_whites == 0);
			uint64_t pinray = checkray * (popcnt(crossfire_whites) == 1);
			pinV |= pinray & Lookup::vertical[friendly_ksq];
			pinFD |= pinray & Lookup::forward_diagonal[friendly_ksq];
			pinH |= pinray & Lookup::horizontal[friendly_ksq];
			pinBD |= pinray & Lookup::back_diagonal[friendly_ksq];
		}

		pin_O = pinV | pinH;
		pin_D = pinFD | pinBD;
		not_pinned = ~(pin_O | pin_D);
		in_double_check = (popcnt(Lookup::double_check[friendly_ksq] & checkmask) > 1);

	}

	void WhiteMoveGenerator::generate() {

		uint64_t pawns = Board::bitboards[WHITE_PAWN] & not_7_or_h & ~(pin_O | pinBD);
		for (uint64_t upright = (pawns << 7) & Board::black_pieces & checkmask; upright; upright = blsr(upright)) {
			int to = tzcnt(upright);
			moves[move_count++] = to - 7 + (to << 6);
		}
		pawns = Board::bitboards[WHITE_PAWN] & not_7_or_a & ~(pin_O | pinFD);
		for (uint64_t upleft = (pawns << 9) & Board::black_pieces & checkmask; upleft; upleft = blsr(upleft)) {
			int to = tzcnt(upleft);
			moves[move_count++] = to - 9 + (to << 6);
		}
		pawns = Board::bitboards[WHITE_PAWN] & ~seventh_rank & ~(pin_D | pinH);
		for (uint64_t push = (pawns << 8) & empty & checkmask; push; push = blsr(push)) {
			int to = tzcnt(push);
			moves[move_count++] = to - 8 + (to << 6);
		}
		uint64_t r = ((third_rank & empty) << 8) & empty;
		pawns = Board::bitboards[WHITE_PAWN] & ~(pin_D | pinH);
		for (uint64_t double_push = (pawns << 16) & r & checkmask; double_push; double_push = blsr(double_push)) {
			int to = tzcnt(double_push);
			moves[move_count++] = to - 16 + (to << 6);
		}
		for (uint64_t from_map = Board::bitboards[WHITE_KNIGHT] & not_pinned; from_map; from_map = blsr(from_map)) {
			int from = tzcnt(from_map);
			for (uint64_t to_map = Lookup::knight_masks[from] & legal_squares; to_map; to_map = blsr(to_map))
				moves[move_count++] = from + (tzcnt(to_map) << 6);
		}
		for (uint64_t from_map = BishopQueen & not_pinned; from_map; from_map = blsr(from_map)) {
			int from = tzcnt(from_map);
			for (uint64_t to_map = BISHOP_ATTACKS(from, occupied) & legal_squares; to_map; to_map = blsr(to_map))
				moves[move_count++] = from + (tzcnt(to_map) << 6);
		}
		for (uint64_t from_map = BishopQueen & ~not_pinned; from_map; from_map = blsr(from_map)) {
			int from = tzcnt(from_map);
			for (uint64_t to_map = BISHOP_ATTACKS(from, occupied) & legal_squares & Lookup::pinmask[friendly_ksq][from]; to_map; to_map = blsr(to_map))
				moves[move_count++] = from + (tzcnt(to_map) << 6);
		}
		for (uint64_t from_map = RookQueen & not_pinned; from_map; from_map = blsr(from_map)) {
			int from = tzcnt(from_map);
			for (uint64_t to_map = ROOK_ATTACKS(from, occupied) & legal_squares; to_map; to_map = blsr(to_map))
				moves[move_count++] = from + (tzcnt(to_map) << 6);
		}
		for (uint64_t from_map = RookQueen & ~not_pinned; from_map; from_map = blsr(from_map)) {
			int from = tzcnt(from_map);
			for (uint64_t to_map = ROOK_ATTACKS(from, occupied) & legal_squares & Lookup::pinmask[friendly_ksq][from]; to_map; to_map = blsr(to_map))
				moves[move_count++] = from + (tzcnt(to_map) << 6);
		}
		for (uint64_t to_map = Lookup::king_masks[friendly_ksq] & ~(Board::white_pieces | seen_by_enemy); to_map; to_map = blsr(to_map))
			moves[move_count++] = friendly_ksq + (tzcnt(to_map) << 6);

	}

	void WhiteMoveGenerator::gen_promotions() {

		uint64_t pawns_on_7 = Board::bitboards[WHITE_PAWN] & seventh_rank;
		uint64_t pawns = pawns_on_7 & ~(hfile | pin_O | pinBD);
		for (uint64_t upright = (pawns << 7) & Board::black_pieces & checkmask; upright; upright = blsr(upright)) {
			int to = tzcnt(upright);
			moves[move_count++] = to - 7 + (to << 6) + promotion_flag;
		}
		pawns = pawns_on_7 & ~(afile | pin_O | pinFD);
		for (uint64_t upleft = (pawns << 9) & Board::black_pieces & checkmask; upleft; upleft = blsr(upleft)) {
			int to = tzcnt(upleft);
			moves[move_count++] = to - 9 + (to << 6) + promotion_flag;
		}
		pawns = pawns_on_7 & not_pinned;
		for (uint64_t push = (pawns << 8) & empty & checkmask; push; push = blsr(push)) {
			int to = tzcnt(push);
			moves[move_count++] = to - 8 + (to << 6) + promotion_flag;
		}

	}

	void WhiteMoveGenerator::gen_ep() {

		uint64_t move_map = (Board::bitboards[WHITE_PAWN] << 7) & GameState::current_ep_square() & sixth_rank;
		if (move_map && !((Board::bitboards[WHITE_KING] & sixth_rank) && ((Board::bitboards[BLACK_QUEEN] | Board::bitboards[BLACK_ROOK]) & sixth_rank))) {
			int move_square = tzcnt(move_map);
			moves[move_count++] = move_square - 7 + (move_square << 6) + ep_flag;
		}
		move_map = (Board::bitboards[WHITE_PAWN] << 9) & GameState::current_ep_square() & sixth_rank;
		if (move_map && !((Board::bitboards[WHITE_KING] & sixth_rank) && ((Board::bitboards[BLACK_QUEEN] | Board::bitboards[BLACK_ROOK]) & sixth_rank))) {
			int move_square = tzcnt(move_map);
			moves[move_count++] = move_square - 9 + (move_square << 6) + ep_flag;
		}

	}

	void WhiteMoveGenerator::gen_castles() {

		if (~checkmask) return;
		if ((!(occupied & 0x6ull)) && !is_attacked(0x6ull) && GameState::rights_K())
			moves[move_count++] = scastle_flag;
		if ((!(occupied & 0x70ull)) && !is_attacked(0x30ull) && GameState::rights_Q())
			moves[move_count++] = lcastle_flag;

	}

	void WhiteMoveGenerator::generate_WK() {

		for (uint64_t to_map = Lookup::king_masks[friendly_ksq] & ~(Board::white_pieces | seen_by_enemy); to_map; to_map = blsr(to_map))
			moves[move_count++] = friendly_ksq + (tzcnt(to_map) << 6);

	}

	bool WhiteMoveGenerator::is_attacked(uint64_t squares) {

		return squares & seen_by_enemy;

	}

	bool WhiteMoveGenerator::incheck() {

		return ~checkmask;

	}

	uint64_t WhiteMoveGenerator::squares_controlled_by_black() {

		uint64_t occ_kingless = occupied ^ (1ull << friendly_ksq);
		uint64_t protected_squares = ((Board::bitboards[BLACK_PAWN] >> 7) & NOT_FILE_H) | ((Board::bitboards[BLACK_PAWN] >> 9) & NOT_FILE_A);

		for (uint64_t piece_map = Board::bitboards[BLACK_KNIGHT]; piece_map; piece_map = blsr(piece_map))
			protected_squares |= Lookup::knight_masks[tzcnt(piece_map)];
		for (uint64_t piece_map = Board::bitboards[BLACK_BISHOP] | Board::bitboards[BLACK_QUEEN]; piece_map; piece_map = blsr(piece_map))
			protected_squares |= BISHOP_ATTACKS(tzcnt(piece_map), occ_kingless);
		for (uint64_t piece_map = Board::bitboards[BLACK_ROOK] | Board::bitboards[BLACK_QUEEN]; piece_map; piece_map = blsr(piece_map))
			protected_squares |= ROOK_ATTACKS(tzcnt(piece_map), occ_kingless);
		return protected_squares | Lookup::king_masks[tzcnt(Board::bitboards[BLACK_KING])];

	}

}
