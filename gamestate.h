
#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "Defs.h"
#include "RepetitionTable.h"
#include <vector>
#include <cstdint>
#include <string>

//		                      KQkqepindx
//		00000000000000000000000000000000  <- boardstate
//		KQkq
//	0000    0000 <- uint8_t castling_rights

namespace Chess {

namespace GameState {
	
	inline RepetitionTable repetition_table;
	inline int boardstate;
	inline int boardstate_save;
	inline uint8_t castling_rights;
	inline int halfmove_clock;
	inline bool endgame;
	inline bool mopup;
	inline int white_to_move;
	inline bool white_computer;
	inline bool white_human;

	inline uint64_t current_ep_square() { return 1ull << (boardstate & lsb_6); }
	inline bool rights_K() { return castling_rights & 0b1000; }
	inline bool rights_Q() { return castling_rights & 0b0100; }
	inline bool rights_k() { return castling_rights & 0b0010; }
	inline bool rights_q() { return castling_rights & 0b0001; }

	void parse_fen(std::string fen);
	inline void init(std::string fen) { parse_fen(fen); }
	void update_gamephase();
	void diagnostic();
	void update_enpassant(int move);
	void update(int move);
	void restore();
	void checkmate(bool white_just_moved);

}

}

#endif
