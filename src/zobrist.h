
#ifndef ZOBRIST_H
#define ZOBRIST_H

#include <cstdint>

namespace Chess {

namespace Zobrist {

	inline uint32_t current_seed;
	inline uint64_t key;
	inline uint64_t hash[13][64];
	inline uint64_t black_to_move;
	void init();
	void update();
	uint32_t random_u32();
	uint64_t random_u64();

}

}

#endif
