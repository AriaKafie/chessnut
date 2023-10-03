
#include "Lookup.h"
#include "Util.h"
#include <cstdint>

namespace Chess {

namespace Lookup {

	void init() {

		init_white_pawnshield_scores();
		init_black_pawnshield_scores();
		init_bishop_attacks();
		init_rook_attacks();
		init_chebyshev_table();
		init_checkmasks();
		init_pinmasks();

	}

	void init_pinmasks() {
		for (int ksq = 0; ksq < 64; ksq++) {
			for (int psq = 0; psq < 64; psq++) {
				if (vertical[ksq] == vertical[psq])
					pinmask[ksq][psq] = vertical[ksq];
				else if (forward_diagonal[ksq] == forward_diagonal[psq])
					pinmask[ksq][psq] = forward_diagonal[ksq];
				else if (horizontal[ksq] == horizontal[psq])
					pinmask[ksq][psq] = horizontal[ksq];
				else if (back_diagonal[ksq] == back_diagonal[psq])
					pinmask[ksq][psq] = back_diagonal[ksq];
				else
					pinmask[ksq][psq] = 0;
			}
		}
	}

	void init_bishop_attacks() {

		for (int square = 0; square < 64; square++) {
			uint64_t mask = bishop_masks[square];
			int bitcount = popcnt(mask);
			int occupancy_permutations = 1 << bitcount;
			for (int count = 0; count < occupancy_permutations; count++) {
				uint64_t occupancy = generate_occupancy(mask, count);
				uint64_t magic_index = bishop_hash_offset[square] + pext(occupancy, bishop_masks[square]);
				bishop_attacks[magic_index] = generate_bishop_map(square, occupancy);
			}
		}

	}

	void init_rook_attacks() {

		for (int square = 0; square < 64; square++) {
			uint64_t mask = rook_masks[square];
			int bitcount = popcnt(mask);
			int occupancy_permutations = 1 << bitcount;
			for (int count = 0; count < occupancy_permutations; count++) {
				uint64_t occupancy = generate_occupancy(mask, count);
				uint64_t magic_index = rook_hash_offset[square] + pext(occupancy, rook_masks[square]);
				rook_attacks[magic_index] = generate_rook_map(square, occupancy);
			}
		}

	}

	void init_white_pawnshield_scores() {

		for (int square = 0; square < 64; square++) {
			uint64_t mask = white_pawnshield[square];
			int bitcount = popcnt(mask);
			int permutations = 1 << bitcount;
			for (int count = 0; count < permutations; count++) {
				uint64_t occupancy = generate_occupancy(mask, count);
				int magic_index = pext(occupancy, white_pawnshield[square]);
				white_pawnshield_scores[square][magic_index] = score_white_pawnshield(square,occupancy);
			}
		}

	}

	void init_black_pawnshield_scores() {

		for (int square = 0; square < 64; square++) {
			uint64_t mask = black_pawnshield[square];
			int bitcount = popcnt(mask);
			int permutations = 1 << bitcount;
			for (int count = 0; count < permutations; count++) {
				uint64_t occupancy = generate_occupancy(mask, count);
				int magic_index = pext(occupancy, black_pawnshield[square]);
				black_pawnshield_scores[square][magic_index] = score_black_pawnshield(square, occupancy);
			}
		}

	}

	int score_black_pawnshield(int kingsq, uint64_t occupancy) {

		using namespace Util;

		// if king on e8/d8, this is generally unsafe
		if (kingsq == d8) return -40;
		if (kingsq == e8) return -10;
		// if king on rank 7->1, it's likely in 
		// danger, regardless of pawn structure
		if (kingsq < h8) return std::max(-50 * (7 - (kingsq / 8)), -200);
		// if king has less than
		// 3 pawns protecting it, it is likely in danger
		if (popcnt(occupancy) < 2) return -90;
		if (popcnt(occupancy) < 3) return -50;

		if (kingsq < f8) { // if king on g8/h8
			int score = -300;
			if (kingsq == h8) {
				if ((occupancy & hfile) == 0) score -= 200;
				if ((occupancy & gfile) == 0) score -= 180;
				if ((occupancy & ffile) == 0) score -= 150;
			}
			if (kingsq == g8) {
				if ((occupancy & hfile) == 0) score -= 200;
				if ((occupancy & gfile) == 0) score -= 200;
				if ((occupancy & ffile) == 0) score -= 200;
			}
			for (;occupancy; occupancy = blsr(occupancy)) {
				int pawn_square = tzcnt(occupancy);
				switch (pawn_square) {
				case h7:
					score += 50;
					break;
				case g7:
					score += 200;
					break;
				case f7:
					score += 250;
					break;
				case h6:
					score += 35;
					break;
				case g6:
					score += 80;
					break;
				case f6:
					score += 20;
					break;
				}
			}
			return score / 4;
		}
		if (kingsq == f8) {
			int score = -300;
			if ((occupancy & gfile) == 0) score -= 150;
			if ((occupancy & ffile) == 0) score -= 200;
			if ((occupancy & efile) == 0) score -= 180;
			for (; occupancy; occupancy = blsr(occupancy)) {
				int pawn_square = tzcnt(occupancy);
				switch (pawn_square) {
				case g7:
					score += 50;
					break;
				case f7:
					score += 200;
					break;
				case e7:
					score += 200;
					break;
				case g6:
					score += 35;
					break;
				case f6:
					score += 80;
					break;
				case e6:
					score += 20;
					break;
				}
			}
			return score / 4;
		}
		if (kingsq == c8) {
			int score = -300;
			if ((occupancy & dfile) == 0) score -= 180;
			if ((occupancy & cfile) == 0) score -= 200;
			if ((occupancy & bfile) == 0) score -= 150;
			for (; occupancy; occupancy = blsr(occupancy)) {
				int pawn_square = tzcnt(occupancy);
				switch (pawn_square) {
				case d7:
					score += 250;
					break;
				case c7:
					score += 200;
					break;
				case b7:
					score += 80;
					break;
				case d6:
					score += 15;
					break;
				case c6:
					score += 25;
					break;
				case b6:
					score += 25;
					break;
				}
			}
			return score / 4;
		}
		if (kingsq == b8 || kingsq == a8) {
			int score = -300;
			if ((occupancy & cfile) == 0) score -= 200;
			if ((occupancy & bfile) == 0) score -= 200;
			if ((occupancy & afile) == 0) score -= 180;
			for (; occupancy; occupancy = blsr(occupancy)) {
				int pawn_square = tzcnt(occupancy);
				switch (pawn_square) {
				case c7:
					score += 200;
					break;
				case b7:
					score += 200;
					break;
				case a7:
					score += 80;
					break;
				case c6:
					score += 15;
					break;
				case b6:
					score += 20;
					break;
				case a6:
					score += 20;
					break;
				}
			}
			return score / 4;
		}

		std::cout << "\nUNSUPPORTED SQUARE IN SCORE_BLACK_PAWNSHIELD";
		std::exit(0);

	}

	int score_white_pawnshield(int kingsq, uint64_t occupancy) {

		using namespace Util;

		// if king on e1/d1, this is generally unsafe
		if (kingsq == d1) return -40;
		if (kingsq == e1) return -10;
		// if king on rank 2->8, it's likely in 
		// danger, regardless of pawn structure
		if (kingsq > a1) return std::max(-50 * (kingsq / 8), -200);
		// if king has few pawns
		// protecting it, it is likely in danger
		if (popcnt(occupancy) < 2) return -90;
		if (popcnt(occupancy) < 3) return -50;

		if (kingsq < f1) { // if king on g1/h1
			int score = -300;
			if (kingsq == h1) {
				if ((occupancy & hfile) == 0) score -= 200;
				if ((occupancy & gfile) == 0) score -= 180;
				if ((occupancy & ffile) == 0) score -= 150;
			}
			if (kingsq == g1) {
				if ((occupancy & hfile) == 0) score -= 200;
				if ((occupancy & gfile) == 0) score -= 200;
				if ((occupancy & ffile) == 0) score -= 200;
			}
			for (;occupancy; occupancy = blsr(occupancy)) {
				int pawn_square = tzcnt(occupancy);
				switch (pawn_square) {
				case h2:
					score += 50;
					break;
				case g2:
					score += 200;
					break;
				case f2:
					score += 250;
					break;
				case h3:
					score += 35;
					break;
				case g3:
					score += 80;
					break;
				case f3:
					score += 20;
					break;
				}
			}
			return score / 4;
		}
		if (kingsq == f1) {
			int score = -300;
			if ((occupancy & gfile) == 0) score -= 150;
			if ((occupancy & ffile) == 0) score -= 200;
			if ((occupancy & efile) == 0) score -= 180;		
			for (;occupancy; occupancy = blsr(occupancy)) {
				int pawn_square = tzcnt(occupancy);
				switch (pawn_square) {
				case g2:
					score += 50;
					break;
				case f2:
					score += 200;
					break;
				case e2:
					score += 200;
					break;
				case g3:
					score += 35;
					break;
				case f3:
					score += 80;
					break;
				case e3:
					score += 20;
					break;
				}
			}
			return score / 4;
		}
		if (kingsq == c1) {
			int score = -300;
			if ((occupancy & dfile) == 0) score -= 180;
			if ((occupancy & cfile) == 0) score -= 200;
			if ((occupancy & bfile) == 0) score -= 150;
			for (;occupancy; occupancy = blsr(occupancy)) {
				int pawn_square = tzcnt(occupancy);
				switch (pawn_square) {
				case d2:
					score += 250;
					break;
				case c2:
					score += 200;
					break;
				case b2:
					score += 80;
					break;
				case d3:
					score += 15;
					break;
				case c3:
					score += 25;
					break;
				case b3:
					score += 25;
					break;
				}
			}
			return score / 4;
		}
		if (kingsq == b1 || kingsq == a1) {
			int score = -300;
			if ((occupancy & cfile) == 0) score -= 200;
			if ((occupancy & bfile) == 0) score -= 200;
			if ((occupancy & afile) == 0) score -= 180;
			for (;occupancy; occupancy = blsr(occupancy)) {
				int pawn_square = tzcnt(occupancy);
				switch (pawn_square) {
				case c2:
					score += 200;
					break;
				case b2:
					score += 200;
					break;
				case a2:
					score += 80;
					break;
				case c3:
					score += 15;
					break;
				case b3:
					score += 20;
					break;
				case a3:
					score += 20;
					break;
				}
			}
			return score / 4;
		}
		
		std::cout << "\nUNSUPPORTED SQUARE IN SCORE_WHITE_PAWNSHIELD";
		std::exit(0);

	}

	void init_checkmasks() {

		for (int i = 0; i < 64; i++) {
			for (int j = 0; j < 64; j++) {
				checkmask[i][j] = generate_checkmask(i, j);
			}
		}

	}

	uint64_t generate_checkmask(int kingsq, int queensq) {

		uint64_t king_north = 0, king_east = 0, king_south = 0, king_west = 0;
		uint64_t queen_north = 0, queen_east = 0, queen_south = 0, queen_west = 0;

		king_north |= MAX_LONG << (8*((kingsq/8)+1));
		if (kingsq/8 == 7) king_north = 0ull;
		for (int i = 0; i < kingsq % 8; i++)
			king_east |= hfile << i;
		king_south |= MAX_LONG >> (8*(8-(kingsq/8)));
		if (kingsq/8 == 0) king_south = 0ull;
		for (int i = 7; i > kingsq % 8; i--)
			king_west |= hfile << i;
		queen_north |= MAX_LONG << (8*((queensq/8)+1));
		if (queensq/8 == 7) queen_north = 0ull;
		for (int i = 0; i < queensq % 8; i++)
			queen_east |= hfile << i;
		queen_south |= MAX_LONG >> (8*(8-(queensq/8)));
		if (queensq/8 == 0) queen_south = 0ull;
		for (int i = 7; i > queensq % 8; i--)
			queen_west |= hfile << i;

		uint64_t kingray_north = vertical[kingsq] & king_north;
		uint64_t kingray_northeast = forward_diagonal[kingsq] & king_north;
		uint64_t kingray_east = horizontal[kingsq] & king_east;
		uint64_t kingray_southeast = back_diagonal[kingsq] & king_east;
		uint64_t kingray_south = vertical[kingsq] & king_south;
		uint64_t kingray_southwest = forward_diagonal[kingsq] & king_south;
		uint64_t kingray_west = horizontal[kingsq] & king_west;
		uint64_t kingray_northwest = back_diagonal[kingsq] & king_west;
		uint64_t queenray_north = vertical[queensq] & queen_north;
		uint64_t queenray_northeast = forward_diagonal[queensq] & queen_north;
		uint64_t queenray_east = horizontal[queensq] & queen_east;
		uint64_t queenray_southeast = back_diagonal[queensq] & queen_east;
		uint64_t queenray_south = vertical[queensq] & queen_south;
		uint64_t queenray_southwest = forward_diagonal[queensq] & queen_south;
		uint64_t queenray_west = horizontal[queensq] & queen_west;
		uint64_t queenray_northwest = back_diagonal[queensq] & queen_west;

		uint64_t checkmask = 0ull;
		checkmask |= (kingray_north) & (queenray_south | (1ull << queensq));
		checkmask |= (kingray_northeast) & (queenray_southwest | (1ull << queensq));
		checkmask |= (kingray_east) & (queenray_west | (1ull << queensq));
		checkmask |= (kingray_southeast) & (queenray_northwest | (1ull << queensq));
		checkmask |= (kingray_south) & (queenray_north | (1ull << queensq));
		checkmask |= (kingray_southwest) & (queenray_northeast | (1ull << queensq));
		checkmask |= (kingray_west) & (queenray_east | (1ull << queensq));
		checkmask |= (kingray_northwest) & (queenray_southeast | (1ull << queensq));
		return checkmask;

	}

	void init_chebyshev_table() {

		for (int i = 0; i < 64; i++) {
			for (int j = 0; j < 64; j++) {
				chebyshev[i][j] = chebyshev_distance_between(i, j);
			}
		}

	}

	int chebyshev_distance_between(int square1, int square2) {

		return std::max(std::abs((square1 >> 3) - (square2 >> 3)), std::abs((square1 & 7) - (square2 & 7)));

	}

	uint64_t generate_bishop_map(int square, uint64_t occupancy) {

		uint64_t piece_map = 1ull << square;
		uint64_t diagonal_map = ((occupancy & forward_diagonal[square]) - (piece_map << 1)) ^ bit_reverse(bit_reverse(occupancy & forward_diagonal[square]) - (bit_reverse(piece_map) << 1));
		uint64_t anti_diagonal_map = ((occupancy & back_diagonal[square]) - (piece_map << 1)) ^ bit_reverse(bit_reverse(occupancy & back_diagonal[square]) - (bit_reverse(piece_map) << 1));
		return (diagonal_map & forward_diagonal[square]) | (anti_diagonal_map & back_diagonal[square]);

	}

	uint64_t generate_rook_map(int square, uint64_t occupancy) {

		uint64_t piece_map = 1ull << square;
		uint64_t horizontal_map = (occupancy - (piece_map << 1)) ^ bit_reverse(bit_reverse(occupancy) - (bit_reverse(piece_map) << 1));
		uint64_t vertical_map = ((occupancy & vertical[square]) - (piece_map << 1)) ^ bit_reverse(bit_reverse(occupancy & vertical[square]) - (bit_reverse(piece_map) << 1));
		return (horizontal_map & horizontal[square]) | (vertical_map & vertical[square]);

	}

	uint64_t bit_reverse(uint64_t num) {

		num = (num >> 32) | (num << 32);
		num = ((num & 0xFFFF0000FFFF0000) >> 16)| ((num & 0x0000FFFF0000FFFF) << 16);
		num = ((num & 0xFF00FF00FF00FF00) >> 8) | ((num & 0x00FF00FF00FF00FF) << 8);
		num = ((num & 0xF0F0F0F0F0F0F0F0) >> 4) | ((num & 0x0F0F0F0F0F0F0F0F) << 4);
		num = ((num & 0xCCCCCCCCCCCCCCCC) >> 2) | ((num & 0x3333333333333333) << 2);
		num = ((num & 0xAAAAAAAAAAAAAAAA) >> 1) | ((num & 0x5555555555555555) << 1);
		return num;

	}

	uint64_t generate_occupancy(uint64_t mask, int iteration) {

		int bitcount = popcnt(mask);
		uint64_t occupancy = 0ull;
		for (int bitpos = 0; bitpos < bitcount; bitpos++) {
			int lsb_index = tzcnt(mask);
			if (iteration & (1 << bitpos))
				occupancy |= 1ull << lsb_index;
			mask = blsr(mask);
		}
		return occupancy;

	}

}

}
