
#ifndef MOVEGENERATOR_H
#define MOVEGENERATOR_H

#include <cstdint>

#define MAX_MOVES 90
#define END 89

namespace Chess {

namespace MoveGenerator {

	void generate_moves(bool ep_enabled, int* movelist);
	int bulk();
	bool incheck();
	inline uint64_t seen_by_enemy;

	static void init();
	static void generate();
	static void gen_king();
	static void gen_ep();
	static void gen_castles();
	static void gen_promotions();
	static uint64_t squares_seen_by_enemy();

}

}

#endif