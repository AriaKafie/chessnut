
#include "bench.h"
#include "util.h"
#include "search.h"
#include "whitecapturegenerator.h"
#include "blackcapturegenerator.h"
#include "repetitiontable.h"
#include "board.h"
#include "gamestate.h"
#include "ui.h"
#include "evaluation.h"
#include "transpositiontable.h"
#include "debug.h"
#include "moveordering.hpp"

#include <chrono>
#include <iostream>

int Bench::node_count;

void Bench::count_nodes(int depth) {

	node_count = 0;
	int node_sum = 0;

	for (std::string fen : fens) {

		node_count = 0;
		std::cout << fen << "  ";
		GameState::init(fen);
		TranspositionTable::clear();
		Search::in_search = true;
		auto start_time = Util::curr_time_millis();
		Move best_move = NULLMOVE;

		for (int d = 1; d <= depth; d++) {
			if (GameState::white_to_move) {

				int alpha = MIN_INT;
        MoveList<WHITE> ml(true);
        ml.sort(best_move, 0);

				for (int i = 0; i < ml.length(); i++) {
					Piece capture = Board::pieces[to_sq(ml[i])];
					uint8_t c_rights = GameState::castling_rights;
					Board::make_legal(ml[i]);
					int eval = Search::search<false>(alpha, MAX_INT, d - 1 - reduction[i], 0);
					if (eval > alpha) {
						alpha = eval;
						best_move = ml[i];
					}
					Board::undo_legal(ml[i], capture);
					GameState::castling_rights = c_rights;
				}
			}
			else {

				int beta = MAX_INT;
        MoveList<BLACK> ml(true);
        ml.sort(best_move, 0);

				for (int i = 0; i < ml.length(); i++) {
					Piece capture = Board::pieces[to_sq(ml[i])];
					uint8_t c_rights = GameState::castling_rights;
					Board::make_legal(ml[i]);
					int eval = Search::search<true>(MIN_INT, beta, d - 1 - reduction[i], 0);
					if (eval < beta) {
						beta = eval;
						best_move = ml[i];
					}
					Board::undo_legal(ml[i], capture);
					GameState::castling_rights = c_rights;
				}
			}
		}

		Search::in_search = false;
		auto duration_ms = Util::curr_time_millis() - start_time;
		std::cout << node_count << "\nnodes searched in " << duration_ms << " ms\n\n";
		/*double nodes_d = node_count;
		double duration_seconds = (double)duration_ms / 1000.0;
		double knps = (nodes_d / duration_seconds) / 1000.0;
		std::cout << "kilonodes per second: " << knps << "\n\n";*/
		node_sum += node_count;

	}
	std::cout << "total nodes: " << node_sum << "\n";
}