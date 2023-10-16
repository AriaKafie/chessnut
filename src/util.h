
#ifndef UTIL_H
#define UTIL_H

#include "defs.h"
#include "lookup.h"
#include "board.h"
#include "ui.h"
#include "gamestate.h"
#include "movegenerator.h"
#include <cstdint>
#include <string>
#include <bitset>
#include <chrono>
#include <iostream>

namespace Chess {

namespace Util {

	inline std::string get_fen() {

		std::string fen = "";
		std::string type_to_char = "PNBRQKpnbrqk ";

		for (int i = 63; i >= 0; i--) {
			if ((i % 8 == 7) && (i != 63)) fen += "/";
			int square_type = Board::piece_types[i];
			if (square_type == NULLPIECE) {
				bool break_twice = false;
				int num_empty_squares = 0;
				for (int j = i; Board::piece_types[j] == NULLPIECE; j--) {
					num_empty_squares++;
					if (j % 8 == 0) {
						fen += std::to_string(num_empty_squares);
						i = j;
						break_twice = true;
						break;
					}
				}
				if (break_twice) {
					continue;
				}
				fen += std::to_string(num_empty_squares);
				i -= (num_empty_squares - 1);
				continue;
			}
			fen += type_to_char[square_type];
		}

		fen += (GameState::white_to_move ? " w " : " b ");

		switch (GameState::castling_rights) {
			case  0: return fen + "- - 0 1";
			case  1: return fen + "q - 0 1";
			case  2: return fen + "k - 0 1";
			case  3: return fen + "kq - 0 1";
			case  4: return fen + "Q - 0 1";
			case  5: return fen + "Qq - 0 1";
			case  6: return fen + "Qk - 0 1";
			case  7: return fen + "Qkq - 0 1";
			case  8: return fen + "K - 0 1";
			case  9: return fen + "Kq - 0 1";
			case 10: return fen + "Kk - 0 1";
			case 11: return fen + "Kkq - 0 1";
			case 12: return fen + "KQ - 0 1";
			case 13: return fen + "KQq - 0 1";
			case 14: return fen + "KQk - 0 1";
			case 15: return fen + "KQkq - 0 1";
		}

		return fen;

	}

	inline void print_binary(uint8_t num) {
	
		std::bitset<sizeof(uint8_t) * 8> binary(num);
		std::cout << binary << std::endl;

	}

	inline void print_moves(int* moves) {

		std::cout << std::dec;
		uint64_t pawn = Board::bitboards[WHITE_PAWN] | Board::bitboards[BLACK_PAWN];
		uint64_t knight = Board::bitboards[WHITE_KNIGHT] | Board::bitboards[BLACK_KNIGHT];
		uint64_t bishop = Board::bitboards[WHITE_BISHOP] | Board::bitboards[BLACK_BISHOP];
		uint64_t rook = Board::bitboards[WHITE_ROOK] | Board::bitboards[BLACK_ROOK];
		uint64_t queen = Board::bitboards[WHITE_QUEEN] | Board::bitboards[BLACK_QUEEN];
		uint64_t king = Board::bitboards[WHITE_KING] | Board::bitboards[BLACK_KING];

		for (int i = 0; i < moves[END]; i++) {
			std::cout << moves[i];
			if (moves[i] == scastle_flag) { std::cout << " {scastle}\n"; continue; }
			else if (moves[i] == lcastle_flag) { std::cout << " {lcastle}\n"; continue; }
			std::string fill = moves[i] < 10000 ? "  " : " ";
			if (moves[i] < 1000) fill = "   ";
			std::string typestring;
			uint64_t piece_mask = 1ull << (moves[i] & lsb_6);
			if (piece_mask & pawn) typestring = " pawn";
			if (piece_mask & knight) typestring = " knight";
			if (piece_mask & bishop) typestring = " bishop";
			if (piece_mask & rook) typestring = " rook";
			if (piece_mask & queen) typestring = " queen";
			if (piece_mask & king) typestring = " king";
			if ((moves[i] >> 12) == 1) typestring = " =Q";
			if ((moves[i] >> 12) == 2) typestring = " enpassant";
			std::cout << fill << "{" << UI::coords[moves[i] & lsb_6] << "," << UI::coords[(moves[i] >> 6) & lsb_6] << typestring << "}\n";
		}
		std::cout << "\n";

	}

	inline void print_moves(int* moves, int move_count) {
	
		std::cout << std::dec;
		uint64_t pawn = Board::bitboards[WHITE_PAWN] | Board::bitboards[BLACK_PAWN];
		uint64_t knight = Board::bitboards[WHITE_KNIGHT] | Board::bitboards[BLACK_KNIGHT];
		uint64_t bishop = Board::bitboards[WHITE_BISHOP] | Board::bitboards[BLACK_BISHOP];
		uint64_t rook = Board::bitboards[WHITE_ROOK] | Board::bitboards[BLACK_ROOK];
		uint64_t queen = Board::bitboards[WHITE_QUEEN] | Board::bitboards[BLACK_QUEEN];
		uint64_t king = Board::bitboards[WHITE_KING] | Board::bitboards[BLACK_KING];

		for (int i = 0; i < move_count; i++) {
			std::cout << moves[i];
			if (moves[i] == scastle_flag) { std::cout << " {scastle}\n"; continue; }
			else if (moves[i] == lcastle_flag) { std::cout << " {lcastle}\n"; continue; }
			std::string fill = moves[i] < 10000 ? "  " : " ";
			if (moves[i] < 1000) fill = "   ";
			std::string typestring;
			uint64_t piece_mask = 1ull << (moves[i] & lsb_6);
			if (piece_mask & pawn) typestring = " pawn";
			if (piece_mask & knight) typestring = " knight";
			if (piece_mask & bishop) typestring = " bishop";
			if (piece_mask & rook) typestring = " rook";
			if (piece_mask & queen) typestring = " queen";
			if (piece_mask & king) typestring = " king";
			if ((moves[i] >> 12) == 1) typestring = " =Q";
			if ((moves[i] >> 12) == 2) typestring = " enpassant";
			std::cout << fill << "{" << UI::coords[moves[i] & lsb_6] << "," << UI::coords[(moves[i] >> 6) & lsb_6] << typestring << "}\n";
		}
		std::cout << "\n";

	}

	unsigned long long curr_time_millis();
	uint64_t string_to_long(std::string str);
	std::string long_to_string(uint64_t num);
	void print_bitboard_and_num(uint64_t bb);

	inline void print_bitboard_and_num(uint64_t bb) {

		std::string line = "+---+---+---+---+---+---+---+---+";
		char c = ' ';
		std::string binary_string = long_to_string(bb);
		int leading_zeros = 64 - binary_string.length();
		std::string padding = "";
		for (int i = 0; i < leading_zeros; i++)
			padding += "0";
		binary_string = padding + binary_string;
		for (int i = 0; i < 64; i++) {
			c = (binary_string[i] == '0') ? ' ' : '@';
			if (i % 8 == 0)
				std::cout << "|\n" << line << "\n";
			std::cout << "| " << c << " ";
		}
		std::cout << "|\n" << line << "\n" << "0x" << std::hex << bb << "ull\n";

	}

	inline unsigned long long curr_time_millis() {

		return std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();

	}

	enum coords {
		h1, g1, f1, e1, d1, c1, b1, a1,
		h2, g2, f2, e2, d2, c2, b2, a2,
		h3, g3, f3, e3, d3, c3, b3, a3,
		h4, g4, f4, e4, d4, c4, b4, a4,
		h5, g5, f5, e5, d5, c5, b5, a5,
		h6, g6, f6, e6, d6, c6, b6, a6,
		h7, g7, f7, e7, d7, c7, b7, a7,
		h8, g8, f8, e8, d8, c8, b8, a8,
	};

	enum color { white, black };

	inline uint64_t string_to_long(std::string str) {

		std::bitset<64>binary(str);
		return binary.to_ullong();

	}

	inline std::string long_to_string(uint64_t num) {

		std::bitset<64>binary(num);
		return binary.to_string();

	}

}

}

#endif
