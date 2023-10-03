
#include "TranspositionTable.h"
#include "Zobrist.h"
#include "GameState.h"
#include <iostream>

namespace Chess {

namespace TranspositionTable {

	int lookup(int depth, int alpha, int beta) {

		//if (disabled) return tt_FAIL;

		Entry* entry = &entries[Zobrist::key % tablesize];

		if (entry->key == Zobrist::key) {
			if (entry->depth >= depth) {
				if (entry->flag == tt_EXACT)
					return entry->eval;
				if ((entry->flag == tt_ALPHA) &&
					(entry->eval <= alpha))
					return alpha;
				if ((entry->flag == tt_BETA) &&
					(entry->eval >= beta))
					return beta;
			}
		}

		return tt_FAIL;

	}

	void disable() {

		disabled = true;

	}

	void enable() {

		disabled = false;

	}

	void record(uint8_t depth, uint8_t flag, int eval, uint16_t best_move) {

		/*if (eval < -90000) {
			std::cout << "depth: " << (int)depth << "\neval: " << eval << "\n";
			std::exit(0);
		}*/
		//if (disabled) return;
		int index = Zobrist::key % tablesize;
		entries[index] = Entry(Zobrist::key, depth, flag, eval, best_move);

	}

	int lookup_move() {

		//if (disabled) return 0;

		return entries[Zobrist::key % tablesize].best_move;

	}

	void clear() {

		occupancy = 0;
		for (int i = 0; i < tablesize; i++) {
			entries[i] = Entry(0,0,0,0,0);
		}

	}

}

}

