
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
					eval = std::min(search<false>(MIN_INT, MAX_INT, depth - 1, 0), 0);
				else {
					eval = search<false>(MIN_INT, MAX_INT, depth - 1, 0);
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
					eval = std::max(search<true>(MIN_INT, MAX_INT, depth - 1, 0), 0);
				else {
					eval = search<true>(MIN_INT, MAX_INT, depth - 1, 0);
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

}

}
