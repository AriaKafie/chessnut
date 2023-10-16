
#ifndef WHITEMOVEGENERATOR_H
#define WHITEMOVEGENERATOR_H

#include <string>
#include <cstdint>

namespace Chess {

	class WhiteMoveGenerator {

	public:
		int moves[90];
		int move_count;
		int friendly_ksq;
		bool ep_enabled;
		void init();
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
		bool is_attacked(uint64_t squares);
		inline bool incheck() { return ~checkmask; }
		WhiteMoveGenerator(bool enable_ep=true);
		uint64_t squares_seen_by_black();
		void generate_WK();
		void gen_ep();
		void gen_castles();
		void gen_promotions();
		void generate();

	};

}

#endif
