
#include "BlackCaptureGenerator.h"
#include "Board.h"
#include "UI.h"
#include "Defs.h"
#include "Lookup.h"
#include <intrin.h>
#include <iostream>

namespace Chess {

BlackCaptureGenerator::BlackCaptureGenerator() {

	move_count = 0;
	init_masks();
	if (checkmask == 0) checkmask = MAX_LONG;
	knight_and_bishop_targets &= checkmask;
	rook_targets &= checkmask;
	queen_targets &= checkmask;

}

void BlackCaptureGenerator::generate_moves() {

	if (in_double_check) {
		generate_BK();
		return;
	}
	generate();
	if (Board::bitboards[BLACK_PAWN] & second_rank) gen_promotions();

}

void BlackCaptureGenerator::init_masks() {

	pinV = 0;
	pinFD = 0;
	pinH = 0;
	pinBD = 0;
	checkmask = 0;

	int kingsq = tzcnt(Board::bitboards[BLACK_KING]);
	checkmask |= Lookup::knight_masks[kingsq] & Board::bitboards[WHITE_KNIGHT];
	checkmask |= Lookup::white_pawn_checkers[kingsq] & Board::bitboards[WHITE_PAWN];
	occupied = Board::black_pieces | Board::white_pieces;
	seen_by_enemy = squares_controlled_by_white();
	empty = ~occupied;

	uint64_t white_unprotected = Board::white_pieces & ~seen_by_enemy;
	knight_and_bishop_targets = (Board::white_pieces & ~Board::bitboards[WHITE_PAWN]) | white_unprotected;
	rook_targets = Board::bitboards[WHITE_ROOK] | Board::bitboards[WHITE_QUEEN] | white_unprotected;
	queen_targets = Board::bitboards[WHITE_QUEEN] | white_unprotected;

	uint64_t checkers = 
	(BISHOP_ATTACKS(kingsq, Board::white_pieces) & (Board::bitboards[WHITE_QUEEN] | Board::bitboards[WHITE_BISHOP])) 
        | (ROOK_ATTACKS(kingsq, Board::white_pieces) & (Board::bitboards[WHITE_QUEEN] | Board::bitboards[WHITE_ROOK]));
	
	for (;checkers; checkers = blsr(checkers)) {
		uint64_t checkray = Lookup::checkmask[kingsq][tzcnt(checkers)];
		uint64_t crossfire_blacks = Board::black_pieces & checkray;
		checkmask |= checkray * (crossfire_blacks == 0);
		uint64_t pinray = checkray * (popcnt(crossfire_blacks) == 1);
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

void BlackCaptureGenerator::generate() {

	uint64_t piece_map = Board::bitboards[BLACK_PAWN] & not_2_or_a & ~(pin_O | pinBD);
	uint64_t move_map = (piece_map >> 7) & Board::white_pieces & checkmask;
	int move_square;
	for (; move_map; move_map = blsr(move_map)) {
		move_square = tzcnt(move_map);
		moves[move_count++] = move_square + 7 + (move_square << 6);
	}
	piece_map = Board::bitboards[BLACK_PAWN] & not_2_or_h & ~(pin_O | pinFD);
	move_map = (piece_map >> 9) & Board::white_pieces & checkmask;
	for (; move_map; move_map = blsr(move_map)) {
		move_square = tzcnt(move_map);
		moves[move_count++] = move_square + 9 + (move_square << 6);
	}
	piece_map = Board::bitboards[BLACK_KNIGHT] & not_pinned;
	int piece_square;
	for (; piece_map; piece_map = blsr(piece_map)) {
		piece_square = tzcnt(piece_map);
		move_map = Lookup::knight_masks[piece_square] & knight_and_bishop_targets;
		for (; move_map; move_map = blsr(move_map))
			moves[move_count++] = piece_square + (tzcnt(move_map) << 6);
	}
	piece_map = Board::bitboards[BLACK_BISHOP] & not_pinned;
	for (; piece_map; piece_map = blsr(piece_map)) {
		piece_square = tzcnt(piece_map);
		move_map = BISHOP_ATTACKS(piece_square, occupied) & knight_and_bishop_targets;
		for (; move_map; move_map = blsr(move_map))
			moves[move_count++] = piece_square + (tzcnt(move_map) << 6);
	}
	piece_map = Board::bitboards[BLACK_BISHOP] & pinFD;
	for (; piece_map; piece_map = blsr(piece_map)) {
		piece_square = tzcnt(piece_map);
		move_map = BISHOP_ATTACKS(piece_square, occupied) & knight_and_bishop_targets & pinFD;
		for (; move_map; move_map = blsr(move_map))
			moves[move_count++] = piece_square + (tzcnt(move_map) << 6);
	}
	piece_map = Board::bitboards[BLACK_BISHOP] & pinBD;
	for (; piece_map; piece_map = blsr(piece_map)) {
		piece_square = tzcnt(piece_map);
		move_map = BISHOP_ATTACKS(piece_square, occupied) & knight_and_bishop_targets & pinBD;
		for (; move_map; move_map = blsr(move_map))
			moves[move_count++] = piece_square + (tzcnt(move_map) << 6);
	}
	piece_map = Board::bitboards[BLACK_ROOK] & not_pinned;
	for (; piece_map; piece_map = blsr(piece_map)) {
		piece_square = tzcnt(piece_map);
		move_map = ROOK_ATTACKS(piece_square, occupied) & rook_targets;
		for (; move_map; move_map = blsr(move_map))
			moves[move_count++] = piece_square + (tzcnt(move_map) << 6);
	}
	piece_map = Board::bitboards[BLACK_ROOK] & pinV;
	for (; piece_map; piece_map = blsr(piece_map)) {
		piece_square = tzcnt(piece_map);
		move_map = ROOK_ATTACKS(piece_square, occupied) & rook_targets & pinV;
		for (; move_map; move_map = blsr(move_map))
			moves[move_count++] = piece_square + (tzcnt(move_map) << 6);
	}
	piece_map = Board::bitboards[BLACK_ROOK] & pinH;
	for (; piece_map; piece_map = blsr(piece_map)) {
		piece_square = tzcnt(piece_map);
		move_map = ROOK_ATTACKS(piece_square, occupied) & rook_targets & pinH;
		for (; move_map; move_map = blsr(move_map))
			moves[move_count++] = piece_square + (tzcnt(move_map) << 6);
	}
	piece_map = Board::bitboards[BLACK_QUEEN] & not_pinned;
	for (; piece_map; piece_map = blsr(piece_map)) {
		piece_square = tzcnt(piece_map);
		move_map = QUEEN_ATTACKS(piece_square, occupied) & queen_targets;
		for (; move_map; move_map = blsr(move_map))
			moves[move_count++] = piece_square + (tzcnt(move_map) << 6);
	}
	piece_map = Board::bitboards[BLACK_QUEEN] & pinFD;
	for (;piece_map; piece_map = blsr(piece_map)) {
		piece_square = tzcnt(piece_map);
		move_map = QUEEN_ATTACKS(piece_square, occupied) & queen_targets & pinFD;
		for (; move_map; move_map = blsr(move_map))
			moves[move_count++] = piece_square + (tzcnt(move_map) << 6);
	}
	piece_map = Board::bitboards[BLACK_QUEEN] & pinBD;
	for (;piece_map; piece_map = blsr(piece_map)) {
		piece_square = tzcnt(piece_map);
		move_map = QUEEN_ATTACKS(piece_square, occupied) & queen_targets & pinBD;
		for (; move_map; move_map = blsr(move_map))
			moves[move_count++] = piece_square + (tzcnt(move_map) << 6);
	}
	piece_map = Board::bitboards[BLACK_QUEEN] & pinV;
	for (;piece_map; piece_map = blsr(piece_map)) {
		piece_square = tzcnt(piece_map);
		move_map = QUEEN_ATTACKS(piece_square, occupied) & queen_targets & pinV;
		for (; move_map; move_map = blsr(move_map))
			moves[move_count++] = piece_square + (tzcnt(move_map) << 6);
	}
	piece_map = Board::bitboards[BLACK_QUEEN] & pinH;
	for (;piece_map; piece_map = blsr(piece_map)) {
		piece_square = tzcnt(piece_map);
		move_map = QUEEN_ATTACKS(piece_square, occupied) & queen_targets & pinH;
		for (; move_map; move_map = blsr(move_map))
			moves[move_count++] = piece_square + (tzcnt(move_map) << 6);
	}
	piece_square = tzcnt(Board::bitboards[BLACK_KING]);
	move_map = Lookup::king_masks[piece_square] & Board::white_pieces & ~seen_by_enemy;
	for (;move_map; move_map = blsr(move_map))
		moves[move_count++] = piece_square + (tzcnt(move_map) << 6);

}

void BlackCaptureGenerator::gen_promotions() {

	uint64_t promotable = Board::bitboards[BLACK_PAWN] & second_rank;
	uint64_t piece_map = promotable & ~(afile | pin_O | pinBD);
	uint64_t move_map = (piece_map >> 7) & Board::white_pieces & checkmask;
	int move_square;
	for (; move_map; move_map = blsr(move_map)) {
		move_square = tzcnt(move_map);
		moves[move_count++] = move_square + 7 + (move_square << 6) + promotion_flag;
	}
	piece_map = promotable & ~(hfile | pin_O | pinFD);
	move_map = (piece_map >> 9) & Board::white_pieces & checkmask;
	for (; move_map; move_map = blsr(move_map)) {
		move_square = tzcnt(move_map);
		moves[move_count++] = move_square + 9 + (move_square << 6) + promotion_flag;
	}

}

void BlackCaptureGenerator::generate_BK() {

	int kingsq = tzcnt(Board::bitboards[BLACK_KING]);
	uint64_t move_map = Lookup::king_masks[kingsq] & Board::white_pieces & ~seen_by_enemy;
	int move_square;
	for (; move_map; move_map = blsr(move_map))
		moves[move_count++] = kingsq + (tzcnt(move_map) << 6);

}

uint64_t BlackCaptureGenerator::squares_controlled_by_white() {

	uint64_t occ_kingless = occupied ^ (1ull << tzcnt(Board::bitboards[BLACK_KING]));
	uint64_t protected_squares = ((Board::bitboards[WHITE_PAWN] << 9) & ~hfile) | ((Board::bitboards[WHITE_PAWN] << 7) & ~afile);

	uint64_t piece_map = Board::bitboards[WHITE_KNIGHT];
	for (;piece_map; piece_map = blsr(piece_map))
		protected_squares |= Lookup::knight_masks[tzcnt(piece_map)];
	piece_map = Board::bitboards[WHITE_BISHOP] | Board::bitboards[WHITE_QUEEN];
	for (;piece_map; piece_map = blsr(piece_map))
		protected_squares |= BISHOP_ATTACKS(tzcnt(piece_map), occ_kingless);
	piece_map = Board::bitboards[WHITE_ROOK] | Board::bitboards[WHITE_QUEEN];
	for (;piece_map; piece_map = blsr(piece_map))
		protected_squares |= ROOK_ATTACKS(tzcnt(piece_map), occ_kingless);
	return protected_squares | Lookup::king_masks[tzcnt(Board::bitboards[WHITE_KING])];

}

}
