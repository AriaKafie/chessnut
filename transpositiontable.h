
#ifndef TRANSPOSITIONTABLE_H
#define TRANSPOSITIONTABLE_H

#include <cstdint>

#define tt_FAIL  -1
#define tt_EXACT  0
#define tt_ALPHA  1
#define tt_BETA   2

// 1 entry = ~16 bytes
// 256 MiB table can store ~16,777,216 entries

namespace Chess {

struct Entry {
	uint64_t key;
	uint8_t depth;
	uint8_t flag;
	int eval;
	uint16_t best_move;
	Entry() : 
		key(0), depth(0), flag(0), eval(0), best_move(0) {}
	Entry(uint64_t key_, uint8_t depth_, uint8_t flag_, int eval_,  uint16_t best_move_) :
		key(key_), depth(depth_), flag(flag_), eval(eval_), best_move(best_move_) {}
};

namespace TranspositionTable {

	inline Entry entries[16777216];
	inline int tablesize = 16777216;
	inline int occupancy = 0;
	inline bool disabled = false;

	int lookup(int depth, int alpha, int beta);
	void record(uint8_t depth, uint8_t flag, int eval, uint16_t best_move);
	int lookup_move();
	void clear();
	void disable();
	void enable();

}

}

#endif
