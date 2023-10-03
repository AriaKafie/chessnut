
#include "Board.h"
#include "Util.h"
#include "GameState.h"
#include "Zobrist.h"
#include <iostream>

namespace Chess {

namespace Board {

void init(std::string fen) {

	load_fen(fen);

}

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

void load_fen(std::string fen) {

	white_pieces = 0ull;
	black_pieces = 0ull;

	for (int i = 0; i < 64; i++) {
		piece_types[i] = NULLPIECE;
		if (i <= 12) bitboards[i] = 0ull;
	}

	int square = 63;
	for (int i = 0; i < fen.length(); i++) {
		if (fen[i] == ' ') break;
		if (fen[i] == '/') continue;
		if (fen[i] < 58) {
			square -= fen[i] - 48;
			continue;
		}
		uint64_t square_bit = 1ull << square;
		if		(fen[i] == 'P') { bitboards[WHITE_PAWN] |= square_bit; white_pieces |= square_bit; piece_types[square] = WHITE_PAWN; }
		else if (fen[i] == 'N') { bitboards[WHITE_KNIGHT] |= square_bit; white_pieces |= square_bit; piece_types[square] = WHITE_KNIGHT; }
		else if (fen[i] == 'B') { bitboards[WHITE_BISHOP] |= square_bit; white_pieces |= square_bit; piece_types[square] = WHITE_BISHOP; }
		else if (fen[i] == 'R') { bitboards[WHITE_ROOK] |= square_bit; white_pieces |= square_bit; piece_types[square] = WHITE_ROOK; }
		else if (fen[i] == 'Q') { bitboards[WHITE_QUEEN] |= square_bit; white_pieces |= square_bit; piece_types[square] = WHITE_QUEEN; }
		else if (fen[i] == 'K') { bitboards[WHITE_KING] |= square_bit; white_pieces |= square_bit; piece_types[square] = WHITE_KING; }
		else if (fen[i] == 'p') { bitboards[BLACK_PAWN] |= square_bit; black_pieces |= square_bit; piece_types[square] = BLACK_PAWN; }
		else if (fen[i] == 'n') { bitboards[BLACK_KNIGHT] |= square_bit; black_pieces |= square_bit; piece_types[square] = BLACK_KNIGHT; }
		else if (fen[i] == 'b') { bitboards[BLACK_BISHOP] |= square_bit; black_pieces |= square_bit; piece_types[square] = BLACK_BISHOP; }
		else if (fen[i] == 'r') { bitboards[BLACK_ROOK] |= square_bit; black_pieces |= square_bit; piece_types[square] = BLACK_ROOK; }
		else if (fen[i] == 'q') { bitboards[BLACK_QUEEN] |= square_bit; black_pieces |= square_bit; piece_types[square] = BLACK_QUEEN; }
		else if (fen[i] == 'k') { bitboards[BLACK_KING] |= square_bit; black_pieces |= square_bit; piece_types[square] = BLACK_KING; }
		square--;
	}

	GameState::parse_fen(fen);

}

}

}
