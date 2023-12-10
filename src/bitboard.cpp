
#include "bitboard.h"
#include <iostream>

Bitboard rook_xray[102400];
Bitboard bishop_xray[5248];
Bitboard rook_attacks[102400];
Bitboard bishop_attacks[5248];
Bitboard bishop_masks[SQUARE_NB];
Bitboard rook_masks[SQUARE_NB];
int rook_hash[SQUARE_NB];
int bishop_hash[SQUARE_NB];

Bitboard double_check[SQUARE_NB];
Bitboard knight_attacks[SQUARE_NB];
Bitboard king_attacks[SQUARE_NB];
Bitboard pawn_attacks[COLOR_NB][SQUARE_NB];
Bitboard check_ray[SQUARE_NB][SQUARE_NB];
Bitboard pin_mask[SQUARE_NB][SQUARE_NB];
Bitboard f_diagonal[SQUARE_NB];
Bitboard b_diagonal[SQUARE_NB];
Bitboard file[SQUARE_NB];
uint8_t square_distance[SQUARE_NB][SQUARE_NB];
uint8_t distance_from_center[SQUARE_NB];
int white_kingshield_scores[SQUARE_NB][1 << 6];
int black_kingshield_scores[SQUARE_NB][1 << 6];
uint8_t castling_pext[1 << 6];
Bitboard white_kingshield[SQUARE_NB];
Bitboard black_kingshield[SQUARE_NB];

void init_magics();
int score_white_kingshield(Square ksq, Bitboard occ);
int score_black_kingshield(Square ksq, Bitboard occ);

void Bitboards::init() {

	for (Square s1 = H1; s1 <= A8; s1++) {
		knight_attacks[s1] = 0;
		king_attacks  [s1] = 0;
		for (Square s2 = H1; s2 <= A8; s2++)
			square_distance[s1][s2] = std::max(file_distance(s1, s2), rank_distance(s1, s2));
	}

	init_magics();

	#define fdiag(s) (square_bb(s) | (BishopAttacks(s, 0) & (mask(s, NORTH_EAST) | mask(s, SOUTH_WEST))))
	#define bdiag(s) (square_bb(s) | (BishopAttacks(s, 0) & (mask(s, NORTH_WEST) | mask(s, SOUTH_EAST))))

	#define md(a, b) (rank_distance(a, b) + file_distance(a, b))

	for (Square s1 = H1; s1 <= A8; s1++) {

		f_diagonal[s1] = fdiag(s1);
		b_diagonal[s1] = bdiag(s1);

		file[s1] = file_bb(s1);
		
		distance_from_center[s1] = std::min(std::min(md(s1, E4), md(s1, E5)), std::min(md(s1, D4), md(s1, D5)));

		for (Square s2 = H1; s2 <= A8; s2++) {
			pin_mask[s1][s2] =
				(fdiag  (s1) & fdiag  (s2))
			  | (bdiag  (s1) & bdiag  (s2))
			  | (rank_bb(s1) & rank_bb(s2))
			  | (file_bb(s1) & file_bb(s2));
		}

		for (Square ksq = H1; ksq <= A8; ksq++) {
			check_ray[ksq][s1] = 0ull;
			for (Direction d : {NORTH_EAST, SOUTH_EAST,
                                SOUTH_WEST, NORTH_WEST}) {
				Bitboard bishop_ray =
					BishopAttacks(ksq, square_bb(s1)) & mask(ksq, d);
				if (bishop_ray & square_bb(s1))
					check_ray[ksq][s1] = bishop_ray;
			}
			for (Direction d : {NORTH, EAST, SOUTH, WEST}) {
				Bitboard rook_ray =
					RookAttacks(ksq, square_bb(s1)) & mask(ksq, d);
				if (rook_ray & square_bb(s1))
					check_ray[ksq][s1] = rook_ray;
			}
		}

        for (Direction d : {NORTH, NORTH_EAST, EAST, SOUTH_EAST,
                            SOUTH, SOUTH_WEST, WEST, NORTH_WEST})
			king_attacks[s1] |= safe_step(s1, d);

        for (Direction d : {NORTHNORTH+EAST, NORTH_EAST+EAST, SOUTH_EAST+EAST, SOUTHSOUTH+EAST,
                            SOUTHSOUTH+WEST, SOUTH_WEST+WEST, NORTH_WEST+WEST, NORTHNORTH+WEST})
			knight_attacks[s1] |= safe_step(s1, d);

		Square sq = 8 * (s1 / 8) + 1;

		white_kingshield[s1] =
			((rank_bb(sq + NORTH) | rank_bb(sq + NORTHNORTH))
			& ~(mask(sq + WEST, WEST)))
			<< std::min(5, std::max(0, (s1 % 8) - 1));

		black_kingshield[s1] =
			((rank_bb(sq + SOUTH) | rank_bb(sq + SOUTHSOUTH))
			& ~(mask(sq + WEST, WEST)))
			<< std::min(5, std::max(0, (s1 % 8) - 1));

		double_check[s1] = king_attacks[s1] | knight_attacks[s1];

		pawn_attacks[WHITE][s1] = PawnAttacks<WHITE>(square_bb(s1));
		pawn_attacks[BLACK][s1] = PawnAttacks<BLACK>(square_bb(s1));
	}

	#undef fdiag
	#undef bdiag
	#undef md

	Bitboard msk = square_bb(H1, E1, A1)
				 | square_bb(H8, E8, A8);
	constexpr uint8_t clearK = 0b0111;
	constexpr uint8_t clearQ = 0b1011;
	constexpr uint8_t cleark = 0b1101;
	constexpr uint8_t clearq = 0b1110;

	int permutations = 1 << popcount(msk);

	for (int p = 0; p < permutations; p++) {

		uint64_t occ = generate_occupancy(msk, p);
		uint8_t rights_mask = 0b1111;

		if ((occ & square_bb(H1)) == 0) rights_mask &= clearK;
		if ((occ & square_bb(E1)) == 0) rights_mask &= clearK & clearQ;
		if ((occ & square_bb(A1)) == 0) rights_mask &= clearQ;
		if ((occ & square_bb(H8)) == 0) rights_mask &= cleark;
		if ((occ & square_bb(E8)) == 0) rights_mask &= cleark & clearq;
		if ((occ & square_bb(A8)) == 0) rights_mask &= clearq;

		castling_pext[pext(occ, msk)] = rights_mask;
	}

	for (Square sq = H1; sq <= A8; sq++) {
		for (int i = 0; i < (1 << popcount(white_kingshield[sq])); i++) {
			Bitboard occ = generate_occupancy(white_kingshield[sq], i);
			white_kingshield_scores[sq][pext(occ, white_kingshield[sq])] = score_white_kingshield(sq, occ);
		}
		for (int i = 0; i < (1 << popcount(black_kingshield[sq])); i++) {
			Bitboard occ = generate_occupancy(black_kingshield[sq], i);
			black_kingshield_scores[sq][pext(occ, black_kingshield[sq])] = score_black_kingshield(sq, occ);
		}
	}

}

int score_white_kingshield(Square ksq, Bitboard occ) {
	constexpr int MAX_SCORE =  50;
	constexpr int MIN_SCORE = -50;
	if (ksq > A1 || (popcount(occ) < 2)) return MIN_SCORE;
	if (ksq == E1 || ksq == D1) return 10;
	int score = 0;
	switch (ksq) {
	case H1:
		if ((occ & FILE_H) == 0) score -= 40;
		if ((occ & FILE_G) == 0) score -= 50;
		if ((occ & FILE_F) == 0) score -= 45;
		for (;occ; pop_lsb(occ)) {
			Square s = lsb(occ);
			switch (s) {
			case H2: score += 10; break;
			case G2: score += 20; break;
			case F2: score += 15; break;
			case H3: score += 5;  break;
			case G3: score += 10; break;
			case F3: score += 5;  break;
			}
		}
		break;
	case G1:
		if ((occ & FILE_H) == 0) score -= 40;
		if ((occ & FILE_G) == 0) score -= 50;
		if ((occ & FILE_F) == 0) score -= 45;
		for (;occ; pop_lsb(occ)) {
			Square s = lsb(occ);
			switch (s) {
			case H2: score += 5;  break;
			case G2: score += 25; break;
			case F2: score += 15; break;
			case F3: score += 5;  break;
			}
		}
		break;
	}
	return std::max(MIN_SCORE, std::min(MAX_SCORE, score));
}

int score_black_kingshield(Square ksq, Bitboard occ) {
	return 0;
}

Bitboard sliding_attacks(PieceType pt, Square sq, Bitboard occupied) {

	Direction rook_dir  [4] = {NORTH,EAST,SOUTH,WEST};
	Direction bishop_dir[4] = {NORTH_EAST,SOUTH_EAST,
							   SOUTH_WEST,NORTH_WEST};
	Bitboard atk = 0;
	for (Direction d : (pt == ROOK) ? rook_dir : bishop_dir) {
		Square s = sq;
		while (safe_step(s, d) && !(square_bb(s) & occupied))
			atk |= square_bb(s += d);
	}
	return atk;

}

void init_magics() {

	for (PieceType pt : {ROOK, BISHOP}) {

		int permutation_sum = 0;

		int*      base   = pt == ROOK ? rook_hash    : bishop_hash;
		Bitboard* masks  = pt == ROOK ? rook_masks   : bishop_masks;
		Bitboard* attack = pt == ROOK ? rook_attacks : bishop_attacks;
		Bitboard* xray   = pt == ROOK ? rook_xray    : bishop_xray;

		for (Square sq = H1; sq <= A8; sq++) {
			base[sq] = permutation_sum;
			Bitboard edges = ((FILE_A | FILE_H) & ~file_bb(sq))
						   | ((RANK_1 | RANK_8) & ~rank_bb(sq));
			Bitboard mask = sliding_attacks(pt, sq, 0) & ~edges;
			masks[sq] = mask;
			int permutations = 1 << popcount(mask);
			permutation_sum += permutations;
			for (int p = 0; p < permutations; p++) {
				Bitboard occupied = generate_occupancy(mask, p);
				Bitboard attacks = sliding_attacks(pt, sq, occupied);
				int hash = pext(occupied, mask);
				attack[base[sq] + hash] = attacks;
				xray[base[sq] + hash] =
					sliding_attacks(pt, sq, occupied ^ (attacks & occupied));
			}
		}
	}
}

std::string bitboard_to_string(Bitboard bb) {
	std::string l, s;
	l = s = "+---+---+---+---+---+---+---+---+\n";
	for (Bitboard sqb = square_bb(A8); sqb; sqb >>= 1) {
		s += (sqb & bb) ? "| @ " : "|   ";
		if (sqb & FILE_H)
			s += "|\n" + l;
	}
	return s + "\n";
}
