
#ifndef MOVESORTER_H
#define MOVESORTER_H

#include "Board.h"
#include "Defs.h"
#include "Lookup.h"
#include <cstdint>
#include <type_traits>

#define winning_capture_bonus 8000
#define losing_capture_bonus 2000
#define killer_bonus 4000

namespace Chess {

namespace MoveSorter {

	struct Killer {

		int moveA;
		int moveB;

		void add(int move) {
			if (move != moveA) {
				moveB = moveA;
				moveA = move;
			}
		}

		bool contains(int move) {
			return (move == moveA) || (move == moveB);
		}

	};

	inline Killer killer_moves[100];
	inline int* moves;
	inline int scores[90];
	inline int priority_move;
	inline uint64_t seen_by_enemy_pawns;
	inline uint64_t seen_by_enemy;

	inline int partition(int low, int high) {

		int pivot = scores[high];
		int i = low - 1;
		for (int j = low; j < high; j++) {
			if (scores[j] >= pivot) {
				i++;
				std::swap(scores[i], scores[j]);
				std::swap(moves[i], moves[j]);
			}
		}
		std::swap(scores[i + 1], scores[high]);
		std::swap(moves[i + 1], moves[high]);
		return i + 1;

	}

	inline void quicksort(int low, int high) {

		if (low < high) {
			int pivot_index = partition(low, high);
			quicksort(low, pivot_index - 1);
			quicksort(pivot_index + 1, high);
		}

	}

	template<bool white>
	inline void sort(int priority_move, int* moves_, int length, uint64_t seen_by_enemy, int ply) {
	
		if constexpr (white)
			seen_by_enemy_pawns = ((Board::bitboards[BLACK_PAWN] >> 7) & NOT_FILE_H) | ((Board::bitboards[BLACK_PAWN] >> 9) & NOT_FILE_A);
		else
			seen_by_enemy_pawns = ((Board::bitboards[WHITE_PAWN] << 9) & NOT_FILE_H) | ((Board::bitboards[WHITE_PAWN] << 7) & NOT_FILE_A);

		moves = moves_;

		for (int i = 0; i < length; i++) {

			if (moves[i] == priority_move) {
				scores[i] = 999999;
				continue;
			}

			int score = 0;
			int from = moves[i] & lsb_6;
			int to = (moves[i] >> 6) & lsb_6;

			if (Board::piece_types[to] != NULLPIECE) {
				int material_difference = Lookup::piece_weights[Board::piece_types[to]] - Lookup::piece_weights[Board::piece_types[from]];
				if ((1ull << to) & seen_by_enemy) {
					score = (material_difference >= 0 ? winning_capture_bonus : losing_capture_bonus) + material_difference;
				}
				else {
					score = winning_capture_bonus + material_difference;
				}
			}

			if ((1ull << to) & seen_by_enemy_pawns) {
				score -= 50;
			}

			if (killer_moves[ply].contains(moves[i])) {
				score += killer_bonus;
			}

			scores[i] = score;

		}

		quicksort(0, length - 1);

	}
	
	inline int qscore(int move) {

		int to = (move >> 6) & lsb_6;
		int score = ((1ull << to) & seen_by_enemy_pawns) ? -500 : 0;
		return score + (Lookup::piece_weights[Board::piece_types[to]] << 1);

	}

	template<bool white>
	inline void qsort(int* moves_, int length) {
	
		if constexpr (white)
			seen_by_enemy_pawns = ((Board::bitboards[BLACK_PAWN] >> 7) & NOT_FILE_H) | ((Board::bitboards[BLACK_PAWN] >> 9) & NOT_FILE_A);
		else
			seen_by_enemy_pawns = ((Board::bitboards[WHITE_PAWN] << 9) & NOT_FILE_H) | ((Board::bitboards[WHITE_PAWN] << 7) & NOT_FILE_A);

		moves = moves_;

		for (int i = 0; i < length; i++)
			scores[i] = qscore(moves[i]);

		quicksort(0, length - 1);

	}
	
}

}

#endif
