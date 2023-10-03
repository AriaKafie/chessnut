
#include "Debug.h"
#include "search.h"
#include "Board.h"
#include "Util.h"
#include "UI.h"
#include "Zobrist.h"
#include "GameState.h"
#include "Defs.h"
#include "Lookup.h"
#include "TranspositionTable.h"
#include "WhiteMoveGenerator.h"
#include "BlackMoveGenerator.h"
#include "MoveSorter.h"
#include "UCI.h"
#include "Bench.h"
#include "Evaluation.h"
#include "movegen.h"
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

namespace Chess {

namespace Debug {

	void go() {
		using namespace Util;
		print_bitboard_and_num(Lookup::pinmask[e4][c6]);

		/*std::ofstream out("pinmask.txt");

		for (int k = 0; k < 64; k++) {
			for (int p = 0; p < 64; p++) {
				print_bitboard_to_file(out, Lookup::pinmask[k][p]);
			}
		}*/

	}

	void test() {

		std::cout << Evaluation::passed_pawn_score() << "\n";

	}

	void load_custom_tt(std::string path) {

		std::cout << "loading tt..\n";
		TranspositionTable::clear();

		std::ifstream tt_in(path);
		if (!tt_in.is_open()) {
			std::cout << "couldn't open custom tt";
			std::exit(0);
		}
		std::string entry; // sample 16776259 14618893867324275779 7 1 -240000 0

		while (std::getline(tt_in, entry)) {
			std::string test;
			int index;
			uint64_t key;
			uint8_t depth;
			uint8_t flag;
			int eval;
			uint16_t best_move;
			size_t space = entry.find(' ');
			index = std::stoi(entry.substr(0,space));
			entry = entry.substr(space+1);
			space = entry.find(' ');
			key = std::stoull(entry.substr(0,space));
			entry = entry.substr(space + 1);
			space = entry.find(' ');
			depth = std::stoi(entry.substr(0, space));
			entry = entry.substr(space + 1);
			space = entry.find(' ');
			flag = std::stoi(entry.substr(0, space));
			entry = entry.substr(space + 1);
			space = entry.find(' ');
			eval = std::stoi(entry.substr(0, space));
			entry = entry.substr(space + 1);
			best_move = std::stoi(entry);
			TranspositionTable::entries[index] = Entry(key, depth, flag, eval, best_move);
		}

	}

	void save_table_image() {

		clear_txt();
		std::ofstream tt_save("tt_save.txt", std::ofstream::app);
		if (!tt_save.is_open()) {
			std::cout << "file write failed";
			std::exit(0);
		}

		std::cout << "\nsaving transposition table (occupancy " << (100 * (((double)TranspositionTable::occupancy) / TranspositionTable::tablesize)) << "%)\n";

		for (int i = 0; i < TranspositionTable::tablesize; i++) {
			Entry* e = &TranspositionTable::entries[i];
			if (e->key == 0) continue;
			UI::clear_line();
			std::cout << (100*((double)i)) / TranspositionTable::tablesize << "%";
			tt_save << i << " " << e->key << " " << (int)e->depth << " " << (int)e->flag << " " << (int)e->eval << " " << (int)e->best_move << "\n";
		}

	}

	void clear_txt() {

		std::ofstream tt_clear("tt_save.txt", std::ofstream::trunc);
		if (!tt_clear.is_open()){
			std::cout << "file clear failed";
			std::exit(0);
		}
		tt_clear.close();

	}

	void boardstatus() {

		std::cout << "\n";
		UI::print_board();
		std::cout << "\n\n";
		for (int square = 63; square >= 0; square--) {
			std::string padding = Board::piece_types[square] < 10 ? ",  " : ", ";
			std::cout << Board::piece_types[square] << padding;
			if (square % 8 == 0) std::cout << "\n";
		}
		std::cout << std::hex << Zobrist::key << "\n\n";

	}

	void TT_status() {

		std::cout << "\noccupancy: " << (((double)TranspositionTable::occupancy / TranspositionTable::tablesize) * 100) << "%\n";

	}

	void print_bitboard_to_file(std::ofstream& o, uint64_t bb) {

		std::string line = "+---+---+---+---+---+---+---+---+";
		char c = ' ';
		std::string binary_string = Util::long_to_string(bb);
		int leading_zeros = 64 - binary_string.length();
		std::string padding = "";
		for (int i = 0; i < leading_zeros; i++)
			padding += "0";
		binary_string = padding + binary_string;
		for (int i = 0; i < 64; i++) {
			c = (binary_string[i] == '0') ? ' ' : '@';
			if (i % 8 == 0)
				o << "|\n" << line << "\n";
			o << "| " << c << " ";
		}
		o << "|\n" << line << "\n";

	}

	void thing(uint64_t mask) {
		std::cout << "0x" << std::hex << mask << "ull,";
		for (int i = 0; i < 6; i++) {
			std::cout << "0x" << std::hex << (mask << i) << "ull,";
		}
		std::cout << "0x" << std::hex << (mask << 5) << "ull,\n";
	}

}

}
