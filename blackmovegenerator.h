#ifndef BlackMoveGenerator_H
#define BlackMoveGenerator_H
#include <string>
#include <cstdint>

namespace Chess {

class BlackMoveGenerator {

public:
	int moves[90];
	int move_count;
	int friendly_ksq;
	bool generate_enpassant;
	bool in_double_check;
	void init();
	uint64_t checkmask;
	uint64_t pinH;
	uint64_t pinV;
	uint64_t pinFD;
	uint64_t pinBD;
	uint64_t pin_O;
	uint64_t pin_D;
	uint64_t not_pinned;
	uint64_t occupied;
	uint64_t empty;
	uint64_t BishopQueen;
	uint64_t RookQueen;
	uint64_t legal_squares;
	uint64_t seen_by_enemy;
	BlackMoveGenerator(bool generate_enpassant_=true);
	void generate_moves();
	inline bool is_attacked(uint64_t squares);
	inline bool incheck() { return ~checkmask; }
	uint64_t squares_controlled_by_white();
	void generate_BK();
	void gen_ep();
	void gen_castles();
	void gen_promotions();
	void generate();

};

}

#endif
