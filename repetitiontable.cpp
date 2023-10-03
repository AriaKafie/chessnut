
#include "RepetitionTable.h"
#include "GameState.h"
#include "WhiteMoveGenerator.h"
#include "BlackMoveGenerator.h"
#include "Board.h"
#include "Zobrist.h"
#include "Util.h"

namespace Chess {

	RepetitionTable::RepetitionTable() {

		for (int i = 0; i < listlength; i++) {
			hashlist[i] = 0;
		}
		list_pointer = 0;

	}

	void RepetitionTable::verify_capacity() {

		if (list_pointer >= listlength) {
			for (int i = 0; i < listlength - 1; i++) {
				hashlist[i] = hashlist[i + 1];
			}
			list_pointer--;
		}

	}

	void RepetitionTable::reset() {

		list_pointer = 0;

	}

	bool RepetitionTable::opponent_can_repeat() {

		if (GameState::white_computer) {
			BlackMoveGenerator g(false);
			g.generate_moves();
			for (int i = 0; i < g.move_count; i++) {
				int capture = Board::piece_types[(g.moves[i]>>6)&lsb_6];
				uint8_t c_rights = GameState::castling_rights;
				Board::makemove<false, false>(g.moves[i]);
				if (occurrences(current_hash()) >= 2) {
					Board::undomove<false, false>(g.moves[i], capture);
					GameState::castling_rights = c_rights;
					return true;
				}
				Board::undomove<false, false>(g.moves[i], capture);
				GameState::castling_rights = c_rights;
			}
			return false;
		}
		else {
			WhiteMoveGenerator g(false);
			g.generate_moves();
			for (int i = 0; i < g.move_count; i++) {
				int capture = Board::piece_types[(g.moves[i]>>6)&lsb_6];
				uint8_t c_rights = GameState::castling_rights;
				Board::makemove<true, false>(g.moves[i]);
				if (occurrences(current_hash()) >= 2) {
					Board::undomove<true, false>(g.moves[i], capture);
					GameState::castling_rights = c_rights;
					return true;
				}
				Board::undomove<true, false>(g.moves[i], capture);
				GameState::castling_rights = c_rights;
			}
			return false;
		}

	}

	void RepetitionTable::push() {

		verify_capacity();
		hashlist[list_pointer++] = current_hash();

	}

	void RepetitionTable::pop() {

		if (list_pointer > 0) list_pointer--;

	}

	uint64_t RepetitionTable::current_hash() {

		return Zobrist::key;

	}

	int RepetitionTable::occurrences(uint64_t hash) {

		int occ = 0;
		for (int i = 0; i < list_pointer; i++)
			if (hashlist[i] == hash) occ++;
		return occ;

	}

	int RepetitionTable::count_last_hash() {

		return occurrences(hashlist[list_pointer - 1]);

	}

	void RepetitionTable::print() {

		std::cout << "\n";
		for (int i = list_pointer - 1; i >= 0; i--) {
			std::cout << std::hex << hashlist[i] << "\n";
		}
		std::cout << "\n";

	}

}
