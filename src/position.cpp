
#include "position.h"
#include "util.h"

#include <cstring>
#include <random>
#include <sstream>

Bitboard bitboards[B_KING + 1];
Piece board[SQUARE_NB];

uint64_t Zobrist::hash[B_KING + 1][SQUARE_NB];

void set_gamephase();

void Position::init() {
  
  std::mt19937_64 rng(221564671644);

  for (Piece pc : { W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
                    B_PAWN, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING })
  {
    for (Square sq = H1; sq <= A8; sq++)
      Zobrist::hash[pc][sq] = rng();
  }

}

void Position::set(const std::string& fen) {

  memset(bitboards, 0ull, 16 * sizeof(Bitboard));
  memset(board, NO_PIECE, 64 * sizeof(Piece   ));

  uint8_t token;
  Square sq = A8;
  size_t piece, idx;

  std::istringstream ss(fen);
  ss >> std::noskipws;

  while (ss >> token) {
    if (std::isdigit(token))
      sq -= token - '0';
    else if (std::isspace(token))
      break;
    else if ((piece = piece_to_char.find(token)) != std::string::npos) {
      board[sq] = piece;
      bitboards[piece] ^= square_bb(sq);
      bitboards[color_of(piece)] ^= square_bb(sq);
      sq--;
    }
  }

  ss >> token;
  side_to_move = token == 'w' ? WHITE : BLACK;
  ss >> token;

  st.castling_rights = 0;
  while (ss >> token)
    if ((idx = std::string("qkQK").find(token)) != std::string::npos)
      st.castling_rights ^= 1 << idx;

  st.key = side_to_move == WHITE ? 0 : Zobrist::Side;

  for (Square sq = H1; sq <= A8; sq++)
    st.key ^= Zobrist::hash[piece_on(sq)][sq];

  set_gamephase();

}

void commit_move(Move m) {
  if (Position::side_to_move == WHITE)
    do_move<WHITE>(m);
  else
    do_move<BLACK>(m);
  piece_stack.clear();
  set_gamephase();
}

void set_gamephase() {

#define piece_count(pc) popcount(bitboards[pc])
  int friendly_material =
    3 * piece_count(make_piece(Position::us, KNIGHT)) +
    3 * piece_count(make_piece(Position::us, BISHOP)) +
    5 * piece_count(make_piece(Position::us, ROOK))   +
    9 * piece_count(make_piece(Position::us, QUEEN));
    
  int enemy_material =
    3 * piece_count(make_piece(Position::them, KNIGHT)) +
    3 * piece_count(make_piece(Position::them, BISHOP)) +
    5 * piece_count(make_piece(Position::them, ROOK))   +
    9 * piece_count(make_piece(Position::them, QUEEN));

  if (enemy_material < 5 && friendly_material >= 5)
    Position::gamephase = MOPUP;
  else if (enemy_material < 10 || enemy_material < 17 && !bitboards[make_piece(Position::them, QUEEN)])
    Position::gamephase = ENDGAME;
  else
    Position::gamephase = MIDGAME;
#undef piece_count

}