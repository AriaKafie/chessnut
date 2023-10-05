
#include "UCI.h"
#include "Defs.h"
#include "Util.h"
#include "UI.h"
#include "Board.h"
#include "Zobrist.h"
#include "GameState.h"
#include "TranspositionTable.h"
#include "search.h"
#include "Perft.h"
#include "Bench.h"
#include "Debug.h"
#include "mouse.h"
#include <iomanip>
#include <cctype>
#include <iostream>

namespace Chess {

namespace UCI {

	void loop() {

		std::string command = "";

		while (command != "quit") {

			std::getline(std::cin, command);
			trim(command);

			if (command == "uci") handle_uci();
			else if (command == "d") handle_d();
			else if (command == "debug") handle_debug();
			else if (command == "isready") handle_isready();
			else if (command == "ucinewgame") handle_newgame();
			else if (starts_with(command, "position")) handle_position(command);
			else if (starts_with(command, "go")) handle_go(command);
			else if (starts_with(command, "gameloop")) handle_gameloop(command);

		}

	}

	void handle_gameloop(std::string input) {
		
		UI::print_board();
		std::string command;

		if (input.find("white") != std::string::npos) {
		
			GameState::white_computer = true;
			GameState::white_human = false;
			
			if (GameState::white_to_move) {
				make_ai_move();
				GameState::checkmate(true);
				std::cin >> command;
				while (command != "quit") {
					move_prompt(command);
					UI::print_board();
					GameState::checkmate(false);
					make_ai_move();
					GameState::checkmate(true);
					std::cin >> command;
				}
				return;
			}
			else {
				std::cin >> command;
				while (command != "quit") {
					move_prompt(command);
					UI::print_board();
					GameState::checkmate(false);
					make_ai_move();
					GameState::checkmate(true);
					std::cin >> command;
				}
				return;
			}
		}
		else {
		
			GameState::white_computer = false;
			GameState::white_human = true;
			
			if (GameState::white_to_move) {
				std::cin >> command;
				while (command != "quit") {
					move_prompt(command);
					UI::print_board();
					GameState::checkmate(true);
					make_ai_move();
					GameState::checkmate(false);
					std::cin >> command;
				}
				return;
			}
			else {
				make_ai_move();
				GameState::checkmate(false);
				std::cin >> command;
				while (command != "quit") {
					move_prompt(command);
					UI::print_board();
					GameState::checkmate(true);
					make_ai_move();
					GameState::checkmate(false);
					std::cin >> command;
				}
				return;
			}
		}

	}

	void make_ai_move() {

		if (GameState::white_to_move) {
			int best_move = UCI_to_int(GameState::white_to_move, Search::probe_white(thinktime));
			Mouse::make_move(best_move);
			Board::make_legal(true, best_move);
			UI::print_board();
			std::cout << Util::get_fen() << "\n";
			std::cout << UI::move_to_string(best_move) << "\ndepth searched: " << std::dec << Debug::last_depth_searched << "\n";
		}
		else {
			int best_move = UCI_to_int(GameState::white_to_move, Search::probe_black(thinktime));
			Mouse::make_move(best_move);
			Board::make_legal(false, best_move);
			UI::print_board();
			std::cout << Util::get_fen() << "\n";
			std::cout << UI::move_to_string(best_move) << "\ndepth searched: " << std::dec << Debug::last_depth_searched << "\n";
		}

	}

	void move_prompt(std::string move) {

		std::string input = move;
		int to_int = UI::movestring_to_int(input);
		
		while (to_int == -1) {
			std::cout << "invalid\n";
			std::cin >> input;
			to_int = UI::movestring_to_int(input);
		}
		
		Board::make_legal(GameState::white_human, to_int);

	}

	void handle_newgame() {

		TranspositionTable::clear();
		GameState::repetition_table.reset();

	}

	void handle_debug() {

		std::cout << std::uppercase;
		GameState::repetition_table.print();
		std::cout << "tt: " << (TranspositionTable::disabled ? "disabled\n" : "enabled\n");
		std::cout << "endgame: " << (GameState::endgame ? "true\n" : "false\n");
		std::cout << "mopup: " << (GameState::mopup ? "true\n" : "false\n");
		std::cout << "last depth searched: " << std::dec << Debug::last_depth_searched << "\n";

	}

	void handle_go(std::string input) {

		if (input.find("eval") != std::string::npos) {
			int depth = std::stoi(input.substr(8));
			int eval = Search::search(MIN_INT, MAX_INT, GameState::white_to_move, depth, 0);
			std::cout << "estimated evaluation at depth " << std::dec << depth << ": " << ((double)eval / 100.0) << "\n";
		}
		else if (input.find("bench") != std::string::npos) {
			input = input.substr(9); //go bench 12 300
			int runs = std::stoi(input.substr(0,input.find(' ')));
			int iterations = std::stoi(input.substr(input.find(' ')+1));
			Bench::go(runs, iterations);
		}
		else if (input.find("perft") != std::string::npos) {
			std::cout << std::dec << "";
			int depth = std::stoi(input.substr(9));
			Perft::go(depth);
		}
		else if (input.find("nodes") != std::string::npos) {
			std::cout << std::dec << "";
			int depth = std::stoi(input.substr(9));
			Bench::count_nodes(depth);
		}
		else if (input.find("debug") != std::string::npos) {
			Debug::go();
		}
		else {
			GameState::white_computer = GameState::white_to_move;
			GameState::white_human = !GameState::white_computer;
			std::string bestmove = GameState::white_to_move ? Search::probe_white(thinktime) : Search::probe_black(thinktime);
			std::cout << "bestmove " << bestmove << "\n";
		}

	}

	void handle_uci () {

		std::cout << "uciok\n";

	}

	void handle_isready () {

		std::cout << "readyok\n";

	}

	void handle_position (std::string input) {

		input = input.substr(9);
		if (starts_with(input, "startpos")) {
			GameState::init(START_FEN);
		}
		else {
			if (input.find("moves") == std::string::npos) {
				GameState::init(input.substr(4));
				Zobrist::update();
				GameState::repetition_table.push();
				return;
			}
			else {
				size_t fen_end = input.find("moves") - 1;
				std::string fen = input.substr(4, fen_end);
				GameState::init(fen);
			}
		}
		Zobrist::update();
		size_t moves_start = input.find("moves");
		if (moves_start != std::string::npos) {
			GameState::repetition_table.reset();
			input = input.substr(input.find("moves") + 6);
			while (input.find(' ') != std::string::npos) {
				std::string uci_move = input.substr(0, input.find(' '));
				int computer_move = UCI_to_int(GameState::white_to_move, uci_move);
				Board::make_legal(GameState::white_to_move, computer_move);
				input = input.substr(input.find(' ') + 1);
			}
			int computer_move = UCI_to_int(GameState::white_to_move, input);
			Board::make_legal(GameState::white_to_move, computer_move);
		}

	}

	std::string int_to_UCI(bool white, int move) {

		int from = move & lsb_6;
		int to = (move >> 6) & lsb_6;
		int movetype = move >> 12;

		if (movetype == SHORTCASTLE) return white ? "e1g1" : "e8g8";
		if (movetype == LONGCASTLE)  return white ? "e1c1" : "e8c8";

		std::string _promotion_flag = (movetype == PROMOTION) ? "q" : "";
		return UI::coords[from] + UI::coords[to] + _promotion_flag;

	}

	int UCI_to_int(bool white_to_move, std::string uci_move) {

		if (uci_move.length() != 4 && uci_move.length() != 5) {
			std::cout << "invalid move in uci_to_int: " << uci_move;
			std::exit(0);
		}

		int promotion_append = 0;
		int enpassant_append = 0;

		if (white_to_move) {
			if (tzcnt(Board::bitboards[WHITE_KING]) == Util::e1) {
				if (uci_move == "e1g1") return scastle_flag;
				if (uci_move == "e1c1") return lcastle_flag;
			}

			if (uci_move.length() == 5) {
				if (uci_move.find('q') == 4) {
					promotion_append = promotion_flag;
				}
				else {
					std::cout << "invalid move in uci_to_int: " << uci_move;
					std::exit(0);
				}
			}

			int from = 0;
			int to = 0;

			for (int square = 0; square < 64; square++) {
				if (UI::coords[square] == uci_move.substr(0, 2))
					from = square;
				if (UI::coords[square] == uci_move.substr(2, 2))
					to = square;
			}

			int computer_move = from + (to << 6);

			if ((Board::piece_types[from] == WHITE_PAWN) && ((std::abs(to - from) % 2) != 0) && (Board::piece_types[to] == NULLPIECE))
				enpassant_append = ep_flag;

			return computer_move + enpassant_append + promotion_append;
		}
		else {
			if (tzcnt(Board::bitboards[BLACK_KING]) == Util::e8) {
				if (uci_move == "e8g8") return scastle_flag;
				if (uci_move == "e8c8") return lcastle_flag;
			}

			if (uci_move.length() == 5) {
				if (uci_move.find('q') == 4) {
					promotion_append = promotion_flag;
				}
				else {
					std::cout << "invalid move in uci_to_int: " << uci_move;
					std::exit(0);
				}
			}

			int from = 0;
			int to = 0;

			for (int square = 0; square < 64; square++) {
				if (UI::coords[square] == uci_move.substr(0, 2))
					from = square;
				if (UI::coords[square] == uci_move.substr(2, 2))
					to = square;
			}

			int computer_move = from + (to << 6);

			if ((Board::piece_types[from] == BLACK_PAWN) && ((std::abs(to - from) % 2) != 0) && (Board::piece_types[to] == NULLPIECE))
				enpassant_append = ep_flag;

			return computer_move + enpassant_append + promotion_append;
		}

	}

	void handle_d() {

		UI::print_board();
		std::cout << "Fen: " << Util::get_fen() << "\n";
		std::cout << "Key: " << std::hex << std::uppercase << Zobrist::key << "\n\n";

	}

	bool starts_with(const std::string& str, const std::string& prefix) {

		return str.compare(0, prefix.length(), prefix) == 0;

	}

	void trim(std::string& str) {

		size_t start = 0;
		size_t end = str.length();

		while (start < end && std::isspace(str[start])) {
			start++;
		}

		while (end > start && std::isspace(str[end - 1])) {
			end--;
		}

		str = str.substr(start, end - start);

	}

}

}
