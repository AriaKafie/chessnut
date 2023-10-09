
#ifndef AI_H
#define AI_H

#include "board.h"
#include "evaluation.h"
#include "transpositiontable.h"
#include "defs.h"
#include "whitemovegenerator.h"
#include "blackmovegenerator.h"
#include "whitecapturegenerator.h"
#include "blackcapturegenerator.h"
#include "movesorter.h"
#include <cstdint>
#include <string>

namespace Chess {

namespace Search {

	std::string probe_white(int thinktime);
	std::string probe_black(int thinktime);

	inline bool in_search;
	inline bool search_cancelled;

	inline bool is_alpha_matescore(int score) {
		return score > 90000;
	}
	inline bool is_beta_matescore(int score) {
		return score < -90000;
	}

	inline constexpr int depth_reduction[90] = {
		0,0,0,0,0,0,0,0,0,0,
		1,1,1,1,1,1,1,1,1,1,
		2,2,2,2,2,2,2,2,2,2,
		3,3,3,3,3,3,3,3,3,3,
		4,4,4,4,4,4,4,4,4,4,
		5,5,5,5,5,5,5,5,5,5,
		6,6,6,6,6,6,6,6,6,6,
		7,7,7,7,7,7,7,7,7,7,
		8,8,8,8,8,8,8,8,8,8,
	};

	template<bool maximizing>
	int quiescence_search(int alpha, int beta) {

		if constexpr (maximizing) {
			int eval = Evaluation::evaluate();
			if (eval >= beta)
				return beta;
			alpha = std::max(alpha, eval);
			int best_eval = MIN_INT;
			WhiteCaptureGenerator g;
			g.generate_moves();
			if (g.move_count == 0) return Evaluation::evaluate();
			MoveSorter::qsort<true>(g.moves, g.move_count);
			for (int i = 0; i < g.move_count; i++) {
				int capture = Board::piece_types[(g.moves[i] >> 6) & lsb_6];
				Board::makemove<true, true>(g.moves[i]);
				eval = quiescence_search<false>(alpha, beta);
				Board::undomove<true, true>(g.moves[i], capture);
				best_eval = std::max(eval, best_eval);
				alpha = std::max(alpha, eval);
				if (beta <= alpha)
					return best_eval;
			}
			return best_eval;
		}
		else {
			int eval = Evaluation::evaluate();
			if (eval <= alpha)
				return alpha;
			beta = std::min(beta, eval);
			int best_eval = MAX_INT;
			BlackCaptureGenerator g;
			g.generate_moves();
			if (g.move_count == 0) return Evaluation::evaluate();
			MoveSorter::qsort<false>(g.moves, g.move_count);
			for (int i = 0; i < g.move_count; i++) {
				int capture = Board::piece_types[(g.moves[i] >> 6) & lsb_6];
				Board::makemove<false, true>(g.moves[i]);
				eval = quiescence_search<true>(alpha, beta);
				Board::undomove<false, true>(g.moves[i], capture);
				best_eval = std::min(eval, best_eval);
				beta = std::min(beta, eval);
				if (beta <= alpha)
					return best_eval;
			}
			return best_eval;
		}

	}

	template<bool maximizing>
	int search(int alpha, int beta, int depth, int ply_from_root) {

		if (depth <= 0) {
			if constexpr (maximizing) 
				return quiescence_search<true >(alpha, beta);
			else 
				return quiescence_search<false>(alpha, beta);
		}

		int trylookup = TranspositionTable::lookup(depth, alpha, beta);
		if (trylookup != tt_FAIL)
			return trylookup;

		int hashflag = tt_ALPHA;

		if constexpr (maximizing) {
			int eval;
			int best_move_yet = 0;
			int best_eval = MIN_INT;
			WhiteMoveGenerator g(ply_from_root == 0);
			g.generate_moves();
			if (g.move_count == 0) return g.incheck() ? (-100000 + ply_from_root * 10) : 0;
			int priority_move = TranspositionTable::lookup_move();
			MoveSorter::sort<true>(priority_move, g.moves, g.move_count, g.seen_by_enemy, ply_from_root);

			for (int i = 0; i < g.move_count; i++) {
				int capture = Board::piece_types[(g.moves[i] >> 6) & lsb_6];
				uint8_t c_rights = GameState::castling_rights;

				Board::makemove<true, false>(g.moves[i]);
				eval = search<false>(alpha, beta, depth - 1 - depth_reduction[i], ply_from_root + 1);
				Board::undomove<true, false>(g.moves[i], capture);

				GameState::castling_rights = c_rights;
				best_eval = std::max(eval, best_eval);

				if (eval > alpha) {
					best_move_yet = g.moves[i];
					alpha = eval;
					hashflag = tt_EXACT;
				}
				if (alpha >= beta) {
					TranspositionTable::record(depth, tt_BETA, beta, g.moves[i]);
					if (capture == NULLPIECE)
						MoveSorter::killer_moves[ply_from_root].add(g.moves[i]);
					return best_eval;
				}
			}

			TranspositionTable::record(depth, hashflag, alpha, best_move_yet);
			return best_eval;
		}
		else {
			int eval;
			int best_move_yet = 0;
			int best_eval = MAX_INT;
			BlackMoveGenerator g(ply_from_root == 0);
			g.generate_moves();
			if (g.move_count == 0) return g.incheck() ? (100000 - ply_from_root * 10) : 0;
			int priority_move = TranspositionTable::lookup_move();
			MoveSorter::sort<false>(priority_move, g.moves, g.move_count, g.seen_by_enemy, ply_from_root);

			for (int i = 0; i < g.move_count; i++) {
				int capture = Board::piece_types[(g.moves[i] >> 6) & lsb_6];
				uint8_t c_rights = GameState::castling_rights;

				Board::makemove<false, false>(g.moves[i]);
				eval = search<true>(alpha, beta, depth - 1 - depth_reduction[i], ply_from_root + 1);
				Board::undomove<false, false>(g.moves[i], capture);

				GameState::castling_rights = c_rights;
				best_eval = std::min(eval, best_eval);

				if (eval < beta) {
					best_move_yet = g.moves[i];
					beta = eval;
					hashflag = tt_EXACT;
				}
				if (beta <= alpha) {
					TranspositionTable::record(depth, tt_BETA, beta, g.moves[i]);
					if (capture == NULLPIECE)
						MoveSorter::killer_moves[ply_from_root].add(g.moves[i]);
					return best_eval;
				}
			}

			TranspositionTable::record(depth, hashflag, beta, best_move_yet);
			return best_eval;
		}
	}

}

}

#endif
