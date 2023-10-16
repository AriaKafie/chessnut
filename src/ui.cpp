
#include "ui.h"
#include "board.h"
#include "whitemovegenerator.h"
#include "blackmovegenerator.h"
#include "util.h"
#include "gamestate.h"
#include "search.h"
#include <string>
#include <iostream>

namespace Chess {

namespace UI {

	std::string move_to_string(int move) {

		if (move == SHORTCASTLE << 12) return "O-O";
		if  (move == LONGCASTLE << 12) return "O-O-O";
		
		int to = (move >> 6) & lsb_6;

		return type_to_char[Board::piece_types[to]] + coords[to];

	}

	int movestring_to_int(std::string move) {
		
		if (move.length() < 4) return -1;
		int from = 0;
		int to = 0;
		
		for (int square = 0; square < 64; square++) {
			if (coords[square] == move.substr(0,2))
				from = square;
			if (coords[square] == move.substr(2,4))
				to = square;
		}
		
		int to_int = from + (to << 6);
		
		if (((Board::piece_types[from] == WHITE_PAWN) ||
		   (Board::piece_types[from] == BLACK_PAWN)) &&
		   ((std::abs(to - from) % 2) != 0) &&
		   (Board::piece_types[to] == NULLPIECE))
		   	to_int += ep_flag;
		if ((Board::piece_types[from] == WHITE_PAWN &&
		   to > 55) || (Board::piece_types[from] == BLACK_PAWN &&
		   to < 8))
		   	to_int += promotion_flag;
		if (move == "scastle") to_int = scastle_flag;
		if (move == "lcastle") to_int = lcastle_flag;
		
		if (GameState::white_human) {
			WhiteMoveGenerator g;
			g.generate_moves();
			for (int i = 0; i < g.move_count; i++)
				if (g.moves[i] == to_int) return to_int;
			return -1;
		}
		else {
			BlackMoveGenerator g;
			g.generate_moves();
			for (int i = 0; i < g.move_count; i++)
				if (g.moves[i] == to_int) return to_int;
			return -1;
		}

	}

	void print_board() {

		std::string line = "+---+---+---+---+---+---+---+---+";
		if (GameState::white_human) {
			std::cout << "\n" << line << "\n| " << type_to_char[Board::piece_types[63]] << " ";
			for (int i = 1; i < 64; i++) {
				if (i % 8 == 0)
					std::cout << "| " << (i % 9) << "\n" << line << "\n";
				std::cout << "| " << type_to_char[Board::piece_types[i^63]] << " ";
			}
			std::cout << "| 1\n" << line << "\n  a   b   c   d   e   f   g   h\n\n";
		}
		else {
			std::cout << "\n" << line << "\n| " << type_to_char[Board::piece_types[0]] << " ";
			for (int i = 1; i < 64; i++) {
				if (i % 8 == 0)
					std::cout << "| " << (9 - (i % 9)) << "\n" << line << "\n";
				std::cout << "| " << type_to_char[Board::piece_types[i]] << " ";
			}
			std::cout << "| 8\n" << line << "\n  h   g   f   e   d   c   b   a\n\n";
		}

	}

	void move_prompt() {

		std::string move;
		std::cout << "enter a move:\n";
		std::cin >> move;
		int to_int = movestring_to_int(move);
		while (to_int == -1) {
			std::cout << "invalid\n";
			std::cin >> move;
			to_int = movestring_to_int(move);
		}
		Board::make_legal(GameState::white_human, to_int);

	}

	void clear_line() {

		for (int i = 0; i < 20; i++) std::cout << "\b";

	}

}

}

