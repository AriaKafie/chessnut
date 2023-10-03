
#include "GameState.h"
#include "Board.h"
#include "Util.h"
#include "WhiteMoveGenerator.h"
#include "BlackMoveGenerator.h"
#include "UI.h"
#include "TranspositionTable.h"
#include "search.h"
#include <iostream>
#include <algorithm>

namespace Chess {

namespace GameState {

	void update(int move) {
	
		repetition_table.push();
		if (!Search::in_search) {
			halfmove_clock++;
			white_to_move ^= 1;
		}
		boardstate_save = boardstate;
		update_enpassant(move);
		update_gamephase();

	}

	void restore() {

		repetition_table.pop();
		boardstate = boardstate_save;
		update_gamephase();

	}

	void checkmate(bool white_just_moved) {

		if (repetition_table.count_last_hash() > 2) {
			std::cout << "draw by repetition\n";
			std::exit(0);
		}
		if (white_just_moved) {
			BlackMoveGenerator g;
			g.generate_moves();
			if (g.move_count == 0) {
				if (g.incheck()) {
					if (white_human) std::cout << "human wins\n";
					else std::cout << "computer wins\n";
					std::exit(0);
				}
				std::cout << "stalemate\n";
				std::exit(0);
			}
		}
		else {
			WhiteMoveGenerator g;
			g.generate_moves();
			if (g.move_count == 0) {
				if (g.incheck()) {
					if (white_human) std::cout << "computer wins\n";
					else std::cout << "human wins\n";
					std::exit(0);
				}
				std::cout << "stalemate\n";
				std::exit(0);
			}
		}

	}

	void diagnostic() {

		Util::print_binary(castling_rights);

		std::cout << "\nK: [" << rights_K();
		std::cout << "]\nQ: [" << rights_Q();
		std::cout << "]\nk: [" << rights_k();
		std::cout << "]\nq: [" << rights_q();
		std::cout << "]\nep [" << (boardstate & lsb_6) << "]\n\n";

	}

	void parse_fen(std::string fen) {

		boardstate = 0;
		castling_rights = 0;
		fen = fen.substr(fen.find(' ') + 1);
		for (int i = 0; i < fen.length(); i++) {
			switch (fen[i]) {
				case ' ':
				case '-':
					continue;
				case 'w':
					white_to_move = 1;
					break;
				case 'b':
					white_to_move = 0;
					break;
				case 'K':
					boardstate |= 1 << 9;
					castling_rights |= 1 << 3;
					break;
				case 'Q':
					boardstate |= 1 << 8;
					castling_rights |= 1 << 2;
					break;
				case 'k':
					boardstate |= 1 << 7;
					castling_rights |= 1 << 1;
					break;
				case 'q':
					boardstate |= 1 << 6;
					castling_rights |= 1;
					break;
			}
		}
		
		update_gamephase();

	}

	void update_gamephase() {

		int enemy_material = 0;
		int friendly_material = 0;

		if (white_computer) {
			enemy_material += 3 * popcnt(Board::bitboards[BLACK_BISHOP] | Board::bitboards[BLACK_KNIGHT]);
			enemy_material += 5 * popcnt(Board::bitboards[BLACK_ROOK]);
			enemy_material += 9 * popcnt(Board::bitboards[BLACK_QUEEN]);
			friendly_material += 3 * popcnt(Board::bitboards[WHITE_BISHOP] | Board::bitboards[WHITE_KNIGHT]);
			friendly_material += 5 * popcnt(Board::bitboards[WHITE_ROOK]);
			friendly_material += 9 * popcnt(Board::bitboards[WHITE_QUEEN]);

			mopup = (enemy_material < 5) && (friendly_material >= 5);
			endgame = (enemy_material < 10) || (enemy_material < 17 && Board::bitboards[BLACK_QUEEN] == 0);

			if (!Search::in_search && mopup)
				TranspositionTable::clear();
		}
		else {
			enemy_material += 3 * popcnt(Board::bitboards[WHITE_BISHOP] | Board::bitboards[WHITE_KNIGHT]);
			enemy_material += 5 * popcnt(Board::bitboards[WHITE_ROOK]);
			enemy_material += 9 * popcnt(Board::bitboards[WHITE_QUEEN]);
			friendly_material += 3 * popcnt(Board::bitboards[BLACK_BISHOP] | Board::bitboards[BLACK_KNIGHT]);
			friendly_material += 5 * popcnt(Board::bitboards[BLACK_ROOK]);
			friendly_material += 9 * popcnt(Board::bitboards[BLACK_QUEEN]);

			mopup = (enemy_material < 5) && (friendly_material >= 5);
			endgame = (enemy_material < 10) || (enemy_material < 17 && Board::bitboards[WHITE_QUEEN] == 0);

			if (!Search::in_search && mopup)
				TranspositionTable::clear();
		}

	}

	void update_enpassant(int move) {

		boardstate &= ~lsb_6; // clear enpassant bits
		int from = move & lsb_6;
		int to = (move >> 6) & lsb_6;
		if (Board::piece_types[to] == WHITE_PAWN || Board::piece_types[to] == BLACK_PAWN) {
			if (from - to == 16)
				boardstate += to + 8;
			if (to - from == 16)
				boardstate += to - 8;
		}

	}

}

}
