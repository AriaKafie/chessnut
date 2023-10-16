
#include "board.h"
#include "util.h"
#include "gamestate.h"
#include "zobrist.h"
#include <iostream>

namespace Chess {

namespace Board {

	void make_legal(bool white, int move) {

		if (white) makemove<true, false>(move);
		else	   makemove<false,false>(move);
		GameState::update(move);

	}

	void undo_legal(bool white, int move, int capture) {

		if (white) undomove<true, false>(move, capture);
		else	   undomove<false,false>(move, capture);
		GameState::restore();

	}

}

}
