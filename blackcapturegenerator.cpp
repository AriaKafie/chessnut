
#include "blackcapturegenerator.h"
#include "board.h"
#include "ui.h"
#include "defs.h"
#include "lookup.h"
#include <iostream>

	BlackCaptureGenerator::BlackCaptureGenerator() :
		move_count(0), pinV(0), pinFD(0), pinH(0), pinBD(0), checkmask(0) {

		init();
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

	void BlackCaptureGenerator::init() {

		friendly_ksq = tzcnt(Board::bitboards[BLACK_KING]);
		checkmask |= KnightAttacks(friendly_ksq) & Board::bitboards[WHITE_KNIGHT];
		checkmask |= Lookup::white_pawn_checkers[friendly_ksq] & Board::bitboards[WHITE_PAWN];
		occupied = Board::black_pieces | Board::white_pieces;
		seen_by_enemy = squares_seen_by_white();
		empty = ~occupied;
		uint64_t white_unprotected = Board::white_pieces & ~seen_by_enemy;
		knight_and_bishop_targets = (Board::white_pieces & ~Board::bitboards[WHITE_PAWN]) | white_unprotected;
		rook_targets = Board::bitboards[WHITE_ROOK] | Board::bitboards[WHITE_QUEEN] | white_unprotected;
		queen_targets = Board::bitboards[WHITE_QUEEN] | white_unprotected;
		uint64_t checkers = 
			(BishopAttacks(friendly_ksq, Board::white_pieces) & (Board::bitboards[WHITE_QUEEN] | Board::bitboards[WHITE_BISHOP])) 
			| (RookAttacks(friendly_ksq, Board::white_pieces) & (Board::bitboards[WHITE_QUEEN] | Board::bitboards[WHITE_ROOK]));

		for (;checkers; checkers = blsr(checkers)) {
			uint64_t checkray_ = CheckRay(friendly_ksq, lsb(checkers));
			uint64_t crossfire_blacks = Board::black_pieces & checkray_;
			checkmask |= checkray_ * (crossfire_blacks == 0);
			uint64_t pinray = checkray_ * (popcount(crossfire_blacks) == 1);
			pinV |= pinray & File(friendly_ksq);
			pinFD |= pinray & FDiag(friendly_ksq);
			pinH |= pinray & Lookup::horizontal[friendly_ksq];
			pinBD |= pinray & BDiag(friendly_ksq);
		}

		pin_O = pinV | pinH;
		pin_D = pinFD | pinBD;
		not_pinned = ~(pin_O | pin_D);
		in_double_check = popcount(DoubleCheck(friendly_ksq) & checkmask) > 1;

	}

	void BlackCaptureGenerator::generate() {

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
		for (uint64_t from_map = Board::bitboards[BLACK_KNIGHT] & not_pinned; from_map; from_map = blsr(from_map)) {
			int from = tzcnt(from_map);
			for (uint64_t to = KnightAttacks(from) & knight_and_bishop_targets; to; to = blsr(to))
				moves[move_count++] = from + (tzcnt(to) << 6);
		}
		for (uint64_t from_map = Board::bitboards[BLACK_BISHOP] & not_pinned; from_map; from_map = blsr(from_map)) {
			int from = tzcnt(from_map);
			for (uint64_t to = BishopAttacks(from, occupied) & knight_and_bishop_targets; to; to = blsr(to))
				moves[move_count++] = from + (tzcnt(to) << 6);
		}
		for (uint64_t from_map = Board::bitboards[BLACK_BISHOP] & pin_D; from_map; from_map = blsr(from_map)) {
			int from = tzcnt(from_map);
			for (uint64_t to = BishopAttacks(from, occupied) & knight_and_bishop_targets & PinMask(friendly_ksq,from); to; to = blsr(to))
				moves[move_count++] = from + (tzcnt(to) << 6);
		}
		for (uint64_t from_map = Board::bitboards[BLACK_ROOK] & not_pinned; from_map; from_map = blsr(from_map)) {
			int from = tzcnt(from_map);
			for (uint64_t to = RookAttacks(from, occupied) & rook_targets; to; to = blsr(to))
				moves[move_count++] = from + (tzcnt(to) << 6);
		}
		for (uint64_t from_map = Board::bitboards[BLACK_ROOK] & pin_O; from_map; from_map = blsr(from_map)) {
			int from = tzcnt(from_map);
			for (uint64_t to = RookAttacks(from, occupied) & rook_targets & PinMask(friendly_ksq,from); to; to = blsr(to))
				moves[move_count++] = from + (tzcnt(to) << 6);
		}
		for (uint64_t from_map = Board::bitboards[BLACK_QUEEN] & not_pinned; from_map; from_map = blsr(from_map)) {
			int from = tzcnt(from_map);
			for (uint64_t to = QueenAttacks(from, occupied) & queen_targets; to; to = blsr(to))
				moves[move_count++] = from + (tzcnt(to) << 6);
		}
		for (uint64_t from_map = Board::bitboards[BLACK_QUEEN] & ~not_pinned; from_map; from_map = blsr(from_map)) {
			int from = tzcnt(from_map);
			for (uint64_t to = QueenAttacks(from, occupied) & queen_targets & PinMask(friendly_ksq,from); to; to = blsr(to))
				moves[move_count++] = from+ (tzcnt(to) << 6);
		}
		for (uint64_t to = KingAttacks(friendly_ksq) & Board::white_pieces & ~seen_by_enemy; to; to = blsr(to))
			moves[move_count++] = friendly_ksq + (tzcnt(to) << 6);

	}

	void BlackCaptureGenerator::gen_promotions() {

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

	}

	void BlackCaptureGenerator::generate_BK() {

		for (uint64_t to = KingAttacks(friendly_ksq) & Board::white_pieces & ~seen_by_enemy; to; to = blsr(to))
			moves[move_count++] = friendly_ksq + (tzcnt(to) << 6);

	}

	uint64_t BlackCaptureGenerator::squares_seen_by_white() {

		uint64_t occ_kingless = occupied ^ (1ull << friendly_ksq);
		uint64_t protected_squares = ((Board::bitboards[WHITE_PAWN] << 9) & NOT_FILE_H) | ((Board::bitboards[WHITE_PAWN] << 7) & NOT_FILE_A);

		for (uint64_t piece_map = Board::bitboards[WHITE_KNIGHT]; piece_map; piece_map = blsr(piece_map))
			protected_squares |= KnightAttacks(tzcnt(piece_map));
		for (uint64_t piece_map = Board::bitboards[WHITE_BISHOP] | Board::bitboards[WHITE_QUEEN]; piece_map; piece_map = blsr(piece_map))
			protected_squares |= BishopAttacks(tzcnt(piece_map), occ_kingless);
		for (uint64_t piece_map = Board::bitboards[WHITE_ROOK] | Board::bitboards[WHITE_QUEEN]; piece_map; piece_map = blsr(piece_map))
			protected_squares |= RookAttacks(tzcnt(piece_map), occ_kingless);
		return protected_squares | KingAttacks(tzcnt(Board::bitboards[WHITE_KING]));

	}
