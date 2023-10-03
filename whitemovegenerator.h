
#ifndef WhiteMoveGenerator_H
#define WhiteMoveGenerator_H
#include <string>
#include <cstdint>

namespace Chess {

class WhiteMoveGenerator {

public:

	int moves[90];
	int move_count;
	bool generate_enpassant;
	void init_masks();
	void generate_moves();
	bool in_double_check;
	uint64_t checkmask;
	uint64_t pinH;
	uint64_t pinV;
	uint64_t pinFD;
	uint64_t pinBD;
	uint64_t pin_O;
	uint64_t pin_D;
	uint64_t not_pinned;
	uint64_t empty;
	uint64_t BishopQueen;
	uint64_t RookQueen;
	uint64_t legal_squares;
	uint64_t occupied;
	uint64_t seen_by_enemy;
	WhiteMoveGenerator(bool generate_enpassant_=true);
	bool is_attacked(uint64_t squares);
	bool incheck();
	uint64_t squares_controlled_by_black();
	void generate_WK();
	void gen_ep();
	void gen_castles();
	void gen_promotions();
	void generate();

};

}

#endif
