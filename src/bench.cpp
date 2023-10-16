
#include "bench.h"
#include "util.h"
#include "search.h"
#include "movegenerator.h"
#include "whitecapturegenerator.h"
#include "blackcapturegenerator.h"
#include "repetitiontable.h"
#include "board.h"
#include "gamestate.h"
#include "ui.h"
#include "evaluation.h"
#include "transpositiontable.h"
#include "movesorter.h"
#include "debug.h"
#include <chrono>
#include <iostream>

namespace Chess {

namespace Bench {

	void warmup() {

		for (int i = 0; i < 200000; i++) {
			for (int j = 1; j < 10000; j++) {
				int k = i / j;
				k += k * ((i / 7)+3);
				if (k == 73) {
					std::cout << "#";
				}
			}
		}

	}

	void count_nodes(int depth) {

		TranspositionTable::clear();
		node_count = 0;
		Search::in_search = true;
		auto start_time = Util::curr_time_millis();

		if (GameState::white_to_move) {
			WhiteMoveGenerator g;
			g.generate_moves();
			for (int i = 0; i < g.move_count; i++) {
				int capture = Board::piece_types[(g.moves[i] >> 6) & lsb_6];
				uint8_t c_rights = GameState::castling_rights;
				Board::make_legal(true, g.moves[i]);
				int eval = Search::search<false>(MIN_INT, MAX_INT, depth - 1, 0);
				Board::undo_legal(true, g.moves[i], capture);
				GameState::castling_rights = c_rights;
			}
		}
		else {
			BlackMoveGenerator g;
			g.generate_moves();
			for (int i = 0; i < g.move_count; i++) {
				int capture = Board::piece_types[(g.moves[i] >> 6) & lsb_6];
				uint8_t c_rights = GameState::castling_rights;
				Board::make_legal(false, g.moves[i]);
				int eval = Search::search<true>(MIN_INT, MAX_INT, depth - 1, 0);
				Board::undo_legal(false, g.moves[i], capture);
				GameState::castling_rights = c_rights;
			}
		}

		Search::in_search = false;
		auto duration_ms = Util::curr_time_millis() - start_time;
		std::cout << "\n" << node_count << " nodes searched in " << duration_ms << " ms\n";
		double nodes_d = node_count;
		double duration_seconds = (double)duration_ms / 1000.0;
		double knps = (nodes_d / duration_seconds) / 1000.0;
		std::cout << "kilonodes per second: " << knps << "\n\n";

	}

	void go(int runs, uint64_t iterations) {

		std::cout << "warming up..\n";
		warmup();
		std::cout << "warmup done\n";

		for (int i = 0; i < runs; i++) {
			auto startTime = Util::curr_time_millis();
			for (int j = 0; j < iterations; j++) {
				WhiteMoveGenerator g(false);
				g.generate_moves();
			}
			timesum += Util::curr_time_millis() - startTime;
			std::cout << timesum / (i+1) << "  [" << i + 1 << "/" << runs << "]\n";
		}
		std::cout << "average: " << timesum / runs;

	}

}

}
