
#include "Evaluation.h"
#include "Util.h"
#include "Board.h"
#include "Lookup.h"
#include "GameState.h"

namespace Chess {

namespace Evaluation {

	int evaluate() {

		return GameState::endgame ? endgame() : midgame();

	}

	int positional_value(int move) {

		int from = move & lsb_6;
		int to = (move >> 6) & lsb_6;
		int movetype = move >> 12;

		if (GameState::white_to_move) {
			if (movetype == SHORTCASTLE || movetype == LONGCASTLE)
				return 35;
			from ^= 63;
			to ^= 63;
			switch (Board::piece_types[to^63]) {
				case WHITE_PAWN:
					return pawn_squares[to] - pawn_squares[from];
				case WHITE_KNIGHT:
					return knight_squares[to] - knight_squares[from];
				case WHITE_BISHOP:
					return bishop_squares[to] - bishop_squares[from];
				case WHITE_ROOK:
					return rook_squares[to] - rook_squares[from];
				case WHITE_QUEEN:
					return queen_squares[to] - queen_squares[from];
				case WHITE_KING:
					return king_squares[to] - king_squares[from];
				default:
					return 0;
			}
		}
		else {
			if (movetype == SHORTCASTLE || movetype == LONGCASTLE)
				return -35;
			switch (Board::piece_types[to]) {
				case BLACK_PAWN:
					return pawn_squares[from] - pawn_squares[to];
				case BLACK_KNIGHT:
					return knight_squares[from] - knight_squares[to];
				case BLACK_BISHOP:
					return bishop_squares[from] - bishop_squares[to];
				case BLACK_ROOK:
					return rook_squares[from] - rook_squares[to];
				case BLACK_QUEEN:
					return queen_squares[from] - queen_squares[to];
				case BLACK_KING:
					return king_squares[from] - king_squares[to];
				default:
					return 0;
			}
		}

	}

	int midgame() {

		return material_count() + king_safety() + pawn_advancement();

	}
	
	int pawn_advancement() {
	
		return 4 * (popcnt(Board::bitboards[WHITE_PAWN] & RANK_567) - popcnt(Board::bitboards[BLACK_PAWN] & RANK_234));
	
	}

	int passed_pawn_score() {
		
		uint64_t black_blocker_mask = (Board::bitboards[BLACK_PAWN] >> 8) | (Board::bitboards[BLACK_PAWN] >> 16);
		black_blocker_mask |= (black_blocker_mask >> 16) | (Board::bitboards[BLACK_PAWN] >> 40);
		black_blocker_mask |= ((black_blocker_mask >> 1) & NOT_FILE_A) | ((black_blocker_mask << 1) & NOT_FILE_H);
		uint64_t white_blocker_mask = (Board::bitboards[WHITE_PAWN] << 8) | (Board::bitboards[WHITE_PAWN] << 16);
		white_blocker_mask |= (white_blocker_mask << 16) | (Board::bitboards[WHITE_PAWN] << 40);
		white_blocker_mask |= ((white_blocker_mask >> 1) & NOT_FILE_A) | ((white_blocker_mask << 1) & NOT_FILE_H);

		return 8 * (popcnt(Board::bitboards[WHITE_PAWN] & ~black_blocker_mask) - popcnt(Board::bitboards[BLACK_PAWN] & ~white_blocker_mask));

	}

	int bishop_mobility() {
	
		int score = 0;
		uint64_t occupied = Board::white_pieces | Board::black_pieces;
		uint64_t friendly_bishops;

		if (GameState::white_computer)
			friendly_bishops = Board::bitboards[WHITE_BISHOP];
		else
			friendly_bishops = Board::bitboards[BLACK_BISHOP];

		for (;friendly_bishops; friendly_bishops = blsr(friendly_bishops)) {
			int square = tzcnt(friendly_bishops);
			score += popcnt(BISHOP_ATTACKS(square, occupied));
		}

		return score;

	}

	int king_safety() {
	
		int white_ksq = tzcnt(Board::bitboards[WHITE_KING]);
		int black_ksq = tzcnt(Board::bitboards[BLACK_KING]);

		return WHITE_KING_SAFETY(white_ksq, Board::bitboards[WHITE_PAWN]) -
		       BLACK_KING_SAFETY(black_ksq, Board::bitboards[BLACK_PAWN]);

	}

	int pawn_structure_score() {

		int score = 0;
		score += 10 * popcnt(Board::bitboards[WHITE_PAWN] & WPsquares_ten);
		score += 6 * popcnt(Board::bitboards[WHITE_PAWN] & WPsquares_six);
		score += 5 * popcnt(Board::bitboards[WHITE_PAWN] & WPsquares_five);
		score += 4 * popcnt(Board::bitboards[WHITE_PAWN] & WPsquares_four);
		score += 2 * popcnt(Board::bitboards[WHITE_PAWN] & WPsquares_two);
		score += popcnt(Board::bitboards[WHITE_PAWN] & WPsquares_one);
		score -= popcnt(Board::bitboards[WHITE_PAWN] & WPsquares_n_one);
		score -= 2 * popcnt(Board::bitboards[WHITE_PAWN] & WPsquares_n_two);
		score -= 4 * popcnt(Board::bitboards[WHITE_PAWN] & WPsquares_n_four);
		return score;

	}

	int endgame() {

		if (GameState::mopup) return mopup();
		
		int score = material_count();

		score += end_king_squares[tzcnt(Board::bitboards[WHITE_KING])];   // incentivize king activity
		score -= end_king_squares[tzcnt(Board::bitboards[BLACK_KING])];
		score += 10 * popcnt(Board::bitboards[WHITE_PAWN] & fourth_rank); // incentivize pawn advancement
		score += 20 * popcnt(Board::bitboards[WHITE_PAWN] & fifth_rank);
		score += 50 * popcnt(Board::bitboards[WHITE_PAWN] & sixth_rank);
		score += 90 * popcnt(Board::bitboards[WHITE_PAWN] & seventh_rank);
		score -= 10 * popcnt(Board::bitboards[BLACK_PAWN] & fifth_rank);
		score -= 20 * popcnt(Board::bitboards[BLACK_PAWN] & fourth_rank);
		score -= 50 * popcnt(Board::bitboards[BLACK_PAWN] & third_rank);
		score -= 90 * popcnt(Board::bitboards[BLACK_PAWN] & second_rank);

		return score;

	}

	int mopup() {

		int score = 0;
		if (GameState::white_computer) {
			score += Lookup::dist_from_center[tzcnt(Board::bitboards[BLACK_KING])] * 10;
			score += (14 - Lookup::chebyshev[tzcnt(Board::bitboards[WHITE_KING])][tzcnt(Board::bitboards[BLACK_KING])]) * 4;
			return score + material_count();
		}
		score -= Lookup::dist_from_center[tzcnt(Board::bitboards[WHITE_KING])] * 10;
		score -= (14 - Lookup::chebyshev[tzcnt(Board::bitboards[WHITE_KING])][tzcnt(Board::bitboards[BLACK_KING])]) * 4;
		return score + material_count();

	}

	int material_count() {

		int score = 100 * (popcnt(Board::bitboards[WHITE_PAWN]) - popcnt(Board::bitboards[BLACK_PAWN]));
		score += 300 * (popcnt(Board::bitboards[WHITE_KNIGHT] | Board::bitboards[WHITE_BISHOP]) - popcnt(Board::bitboards[BLACK_KNIGHT] | Board::bitboards[BLACK_BISHOP]));
		score += 500 * (popcnt(Board::bitboards[WHITE_ROOK]) - popcnt(Board::bitboards[BLACK_ROOK]));
		score += 900 * (popcnt(Board::bitboards[WHITE_QUEEN]) - popcnt(Board::bitboards[BLACK_QUEEN]));
		return score;

	}

}

}
