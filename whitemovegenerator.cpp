
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
		init_masks();
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

	void WhiteMoveGenerator::init_masks() {

		pinV = 0;
		pinFD = 0;
		pinH = 0;
		pinBD = 0;
		checkmask = 0;

		int kingsq = tzcnt(Board::bitboards[WHITE_KING]);
		checkmask |= Lookup::knight_masks[kingsq] & Board::bitboards[BLACK_KNIGHT];
		checkmask |= Lookup::black_pawn_checkers[kingsq] & Board::bitboards[BLACK_PAWN];
		BishopQueen = Board::bitboards[WHITE_BISHOP] | Board::bitboards[WHITE_QUEEN];
		RookQueen = Board::bitboards[WHITE_ROOK] | Board::bitboards[WHITE_QUEEN];

		occupied = Board::black_pieces | Board::white_pieces;
		empty = ~occupied;
		seen_by_enemy = squares_controlled_by_black();

		uint64_t checkers = 
		(BISHOP_ATTACKS(kingsq, Board::black_pieces) & (Board::bitboards[BLACK_QUEEN] | Board::bitboards[BLACK_BISHOP])) 
		| (ROOK_ATTACKS(kingsq, Board::black_pieces) & (Board::bitboards[BLACK_QUEEN] | Board::bitboards[BLACK_ROOK]));
		
		for (;checkers; checkers = blsr(checkers)) {
			uint64_t checkray = Lookup::checkmask[kingsq][tzcnt(checkers)];
			uint64_t crossfire_whites = Board::white_pieces & checkray;
			checkmask |= checkray * (crossfire_whites == 0);
			uint64_t pinray = checkray * (popcnt(crossfire_whites) == 1);
			pinV  |= pinray & Lookup::vertical[kingsq];
			pinFD |= pinray & Lookup::forward_diagonal[kingsq];
			pinH  |= pinray & Lookup::horizontal[kingsq];
			pinBD |= pinray & Lookup::back_diagonal[kingsq];
		}

		pin_O = pinV | pinH;
		pin_D = pinFD | pinBD;
		not_pinned = ~(pin_O | pin_D);
		in_double_check = (popcnt(Lookup::double_check[kingsq] & checkmask) > 1);

	}

	void WhiteMoveGenerator::generate() {

		uint64_t piece_map = Board::bitboards[WHITE_PAWN] & not_7_or_h & ~(pin_O | pinBD);
		uint64_t move_map = (piece_map << 7) & Board::black_pieces & checkmask;
		int move_square;
		for (;move_map; move_map = blsr(move_map)) {
			move_square = tzcnt(move_map);
			moves[move_count++] = move_square - 7 + (move_square << 6);
		}
		piece_map = Board::bitboards[WHITE_PAWN] & not_7_or_a & ~(pin_O | pinFD);
		move_map = (piece_map << 9) & Board::black_pieces & checkmask;
		for (;move_map; move_map = blsr(move_map)) {
			move_square = tzcnt(move_map);
			moves[move_count++] = move_square - 9 + (move_square << 6);
		}
		piece_map = Board::bitboards[WHITE_PAWN] & ~seventh_rank & ~(pin_D | pinH);
		move_map = (piece_map << 8) & empty & checkmask;
		for (;move_map; move_map = blsr(move_map)) {
			move_square = tzcnt(move_map);
			moves[move_count++] = move_square - 8 + (move_square << 6);
		}
		move_map = (piece_map << 8) & empty;
		move_map = (move_map << 8) & fourth_rank & empty & checkmask;
		for (;move_map; move_map = blsr(move_map)) {
			move_square = tzcnt(move_map);
			moves[move_count++] = move_square - 16 + (move_square << 6);
		}
		piece_map = Board::bitboards[WHITE_KNIGHT] & not_pinned;
		int piece_square;
		for (;piece_map; piece_map = blsr(piece_map)) {
			piece_square = tzcnt(piece_map);
			move_map = Lookup::knight_masks[piece_square] & legal_squares;
			for (;move_map; move_map = blsr(move_map))
				moves[move_count++] = piece_square + (tzcnt(move_map) << 6);
		}
		piece_map = BishopQueen & not_pinned;
		for (;piece_map; piece_map = blsr(piece_map)) {
			piece_square = tzcnt(piece_map);
			move_map = BISHOP_ATTACKS(piece_square, occupied) & legal_squares;
			for (;move_map; move_map = blsr(move_map))
				moves[move_count++] = piece_square + (tzcnt(move_map) << 6);
		}
		piece_map = BishopQueen & pinFD;
		for (;piece_map; piece_map = blsr(piece_map)) {
			piece_square = tzcnt(piece_map);
			move_map = BISHOP_ATTACKS(piece_square, occupied) & legal_squares & pinFD;
			for (;move_map; move_map = blsr(move_map))
				moves[move_count++] = piece_square + (tzcnt(move_map) << 6);
		}
		piece_map = BishopQueen & pinBD;
		for (;piece_map; piece_map = blsr(piece_map)) {
			piece_square = tzcnt(piece_map);
			move_map = BISHOP_ATTACKS(piece_square, occupied) & legal_squares & pinBD;
			for (;move_map; move_map = blsr(move_map))
				moves[move_count++] = piece_square + (tzcnt(move_map) << 6);
		}
		piece_map = RookQueen & not_pinned;
		for (;piece_map; piece_map = blsr(piece_map)) {
			piece_square = tzcnt(piece_map);
			move_map = ROOK_ATTACKS(piece_square, occupied) & legal_squares;
			for (;move_map; move_map = blsr(move_map))
				moves[move_count++] = piece_square + (tzcnt(move_map) << 6);
		}
		piece_map = RookQueen & pinV;
		for (;piece_map; piece_map = blsr(piece_map)) {
			piece_square = tzcnt(piece_map);
			move_map = ROOK_ATTACKS(piece_square, occupied) & legal_squares & pinV;
			for (;move_map; move_map = blsr(move_map))
				moves[move_count++] = piece_square + (tzcnt(move_map) << 6);
		}
		piece_map = RookQueen & pinH;
		for (;piece_map; piece_map = blsr(piece_map)) {
			piece_square = tzcnt(piece_map);
			move_map = ROOK_ATTACKS(piece_square, occupied) & legal_squares & pinH;
			for (;move_map; move_map = blsr(move_map))
				moves[move_count++] = piece_square + (tzcnt(move_map) << 6);
		}
		piece_square = tzcnt(Board::bitboards[WHITE_KING]);
		move_map = Lookup::king_masks[piece_square] & ~(Board::white_pieces | seen_by_enemy);
		for (;move_map; move_map = blsr(move_map))
			moves[move_count++] = piece_square + (tzcnt(move_map) << 6);

	}

	void WhiteMoveGenerator::gen_promotions() {

		uint64_t promotable = Board::bitboards[WHITE_PAWN] & seventh_rank;
		uint64_t piece_map = promotable & ~(hfile | pin_O | pinBD);
		uint64_t move_map = (piece_map << 7) & Board::black_pieces & checkmask;
		int move_square;
		for (; move_map; move_map = blsr(move_map)) {
			move_square = tzcnt(move_map);
			moves[move_count++] = move_square - 7 + (move_square << 6) + promotion_flag;
		}
		piece_map = promotable & ~(afile | pin_O | pinFD);
		move_map = (piece_map << 9) & Board::black_pieces & checkmask;
		for (; move_map; move_map = blsr(move_map)) {
			move_square = tzcnt(move_map);
			moves[move_count++] = move_square - 9 + (move_square << 6) + promotion_flag;
		}
		piece_map = promotable & not_pinned;
		move_map = (piece_map << 8) & empty & checkmask;
		for (;move_map; move_map = blsr(move_map)) {
			move_square = tzcnt(move_map);
			moves[move_count++] = move_square - 8 + (move_square << 6) + promotion_flag;
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

		int kingsq = tzcnt(Board::bitboards[WHITE_KING]);
		uint64_t move_map = Lookup::king_masks[kingsq] & ~(Board::white_pieces | seen_by_enemy);
		int move_square;
		for (; move_map; move_map = blsr(move_map)) {
			move_square = tzcnt(move_map);
			moves[move_count++] = kingsq + (move_square << 6);
		}

	}

	bool WhiteMoveGenerator::is_attacked(uint64_t squares) {

		return squares & seen_by_enemy;

	}

	bool WhiteMoveGenerator::incheck() {

		return ~checkmask;

	}

	uint64_t WhiteMoveGenerator::squares_controlled_by_black() {

		uint64_t occ_kingless = occupied ^ (1ull << tzcnt(Board::bitboards[WHITE_KING]));
		uint64_t protected_squares = ((Board::bitboards[BLACK_PAWN] >> 7) & ~hfile) | ((Board::bitboards[BLACK_PAWN] >> 9) & ~afile);

		uint64_t piece_map = Board::bitboards[BLACK_KNIGHT];
		for (;piece_map; piece_map = blsr(piece_map))
			protected_squares |= Lookup::knight_masks[tzcnt(piece_map)];
		piece_map = Board::bitboards[BLACK_BISHOP] | Board::bitboards[BLACK_QUEEN];
		for (;piece_map; piece_map = blsr(piece_map))
			protected_squares |= BISHOP_ATTACKS(tzcnt(piece_map), occ_kingless);
		piece_map = Board::bitboards[BLACK_ROOK] | Board::bitboards[BLACK_QUEEN];
		for (;piece_map; piece_map = blsr(piece_map))
			protected_squares |= ROOK_ATTACKS(tzcnt(piece_map), occ_kingless);
		return protected_squares | Lookup::king_masks[tzcnt(Board::bitboards[BLACK_KING])];

	}

}
