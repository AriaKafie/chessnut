
#include "search.h"
#include "board.h"
#include "ui.h"
#include "whitemovegenerator.h"
#include "blackmovegenerator.h"
#include "whitecapturegenerator.h"
#include "blackcapturegenerator.h"
#include "evaluation.h"
#include "util.h"
#include "gamestate.h"
#include "transpositiontable.h"
#include "debug.h"
#include "bench.h"
#include "movesorter.h"
#include "uci.h"
#include <iostream>
#include <cassert>

namespace Chess {

namespace Search {

	std::string probe_white(int thinktime) {

		in_search = true;
		search_cancelled = false;
		int eval;
		int alpha;
		int best_move = NULLMOVE;
		WhiteMoveGenerator g;
		g.generate_moves();
		auto start_time = Util::curr_time_millis();

		for (int depth = 1; !search_cancelled; depth++) {

			alpha = MIN_INT;
			MoveSorter::sort<true>(best_move, g.moves, g.move_count, g.seen_by_enemy, 0);

			for (int i = 0; i < g.move_count; i++) {

				if ((Util::curr_time_millis() - start_time) > thinktime) {
					search_cancelled = true;
					Debug::last_depth_searched = depth - 1;
					break;
				}

				int capture = Board::piece_types[(g.moves[i] >> 6) & lsb_6];
				uint8_t c_rights = GameState::castling_rights;
				Board::make_legal(true, g.moves[i]);

				if (GameState::repetition_table.count_last_hash() > 2)
					eval = 0;
				else if (GameState::repetition_table.opponent_can_repeat())
					eval = std::min(search(MIN_INT, MAX_INT, false, depth - 1, 0), 0);
				else {
					eval = search(MIN_INT, MAX_INT, false, depth - 1, 0);
					if (!GameState::endgame) eval += Evaluation::positional_value(g.moves[i]);
				}

				Board::undo_legal(true, g.moves[i], capture);
				GameState::castling_rights = c_rights;

				if (eval > alpha) {
					alpha = eval;
					best_move = g.moves[i];
					if (is_alpha_matescore(eval)) {
						search_cancelled = true;
						Debug::last_depth_searched = depth - 1;
						break;
					}
				}

			}
		}

		in_search = false;
		search_cancelled = false;
		return UCI::int_to_UCI(true, (best_move == NULLMOVE) ? g.moves[0] : best_move);

	}

	std::string probe_black(int thinktime) {

		in_search = true;
		search_cancelled = false;
		int eval;
		int beta;
		int best_move = NULLMOVE;
		BlackMoveGenerator g;
		g.generate_moves();
		auto start_time = Util::curr_time_millis();

		for (int depth = 1; !search_cancelled; depth++) {
		
			beta = MAX_INT;
			MoveSorter::sort<false>(best_move, g.moves, g.move_count, g.seen_by_enemy, 0);

			for (int i = 0; i < g.move_count; i++) {

				if ((Util::curr_time_millis() - start_time) > thinktime) {
					search_cancelled = true;
					Debug::last_depth_searched = depth - 1;
					break;
				}

				int capture = Board::piece_types[(g.moves[i] >> 6) & lsb_6];
				uint8_t c_rights = GameState::castling_rights;
				Board::make_legal(false, g.moves[i]);

				if (GameState::repetition_table.count_last_hash() > 2)
					eval = 0;
				else if (GameState::repetition_table.opponent_can_repeat())
					eval = std::max(search(MIN_INT, MAX_INT, true, depth - 1, 0), 0);
				else {
					eval = search(MIN_INT, MAX_INT, true, depth - 1, 0);
					if (!GameState::endgame) eval += Evaluation::positional_value(g.moves[i]);
				}

				Board::undo_legal(false, g.moves[i], capture);
				GameState::castling_rights = c_rights;

				if (eval < beta) {
					beta = eval;
					best_move = g.moves[i];
					if (is_beta_matescore(eval)) {
						search_cancelled = true;
						Debug::last_depth_searched = depth - 1;
						break;
					}
				}

			}
		}

		in_search = false;
		search_cancelled = false;
		return UCI::int_to_UCI(false, (best_move == NULLMOVE) ? g.moves[0] : best_move);

	}

	int search(int alpha, int beta, bool maximizing, int depth, int ply_from_root) {

		if (depth <= 0) return quiescence_search(alpha, beta, maximizing);

		int trylookup = TranspositionTable::lookup(depth, alpha, beta);
		if (trylookup != tt_FAIL)
			return trylookup;

		int hashflag = tt_ALPHA;

		if (maximizing) {
			int best_move_yet = 0;
			int eval;
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
				eval = search(alpha, beta, false, depth - 1 - depth_reduction[i], ply_from_root + 1);
				Board::undomove<true, false>(g.moves[i], capture);
				
				GameState::castling_rights = c_rights;
				best_eval = std::max(eval, best_eval);
				
				if (eval > alpha) {
					best_move_yet = g.moves[i];
					alpha = eval;
					hashflag = tt_EXACT;
				}
				if (beta <= alpha) {
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
			int best_move_yet = 0;
			int eval;
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
				eval = search(alpha, beta, true, depth - 1 - depth_reduction[i], ply_from_root + 1);
				Board::undomove<false, false>(g.moves[i], capture);
				
				GameState::castling_rights = c_rights;
				best_eval = std::min(eval, best_eval);
				
				if (eval < beta) {
					best_move_yet = g.moves[i];
					beta = eval;
					hashflag = tt_EXACT;
				}
				beta = std::min(beta, eval);
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

	int quiescence_search(int alpha, int beta, bool maximizing) {

		if (maximizing) {
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
				eval = quiescence_search(alpha, beta, false);
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
				eval = quiescence_search(alpha, beta, true);
				Board::undomove<false, true>(g.moves[i], capture);
				best_eval = std::min(eval, best_eval);
				beta = std::min(beta, eval);
				if (beta <= alpha)
					return best_eval;
			}
			return best_eval;
		}

	}

}

}
