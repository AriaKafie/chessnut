
#include "position.h"
#include "util.h"

#include <cstring>
#include <random>
#include <sstream>

void set_gamephase();

Bitboard bitboards[B_KING + 1];
Piece board[SQUARE_NB];

uint64_t Zobrist::hash[B_KING + 1][SQUARE_NB];

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

  memset(board, NO_PIECE, 64 * sizeof(Piece));
  memset(bitboards, 0ull, 16 * sizeof(Bitboard));

  uint8_t token;
  Square sq = A8;
  size_t piece, idx;

  std::istringstream iss(fen);
  iss >> std::noskipws;

  while (iss >> token) {
    if (std::isspace(token))
      break;
    if (std::isdigit(token))
      sq -= token - '0';
    else if ((piece = piece_to_char.find(token)) != std::string::npos) {
      board[sq] = piece;
      bitboards[piece] ^= square_bb(sq);
      bitboards[color_of(piece)] ^= square_bb(sq);
      sq--;
    }
  }

  iss >> std::skipws;
  iss >> token;
  side_to_move = token == 'w' ? WHITE : BLACK;

  state_ptr->castling_rights = 0;
  while (iss >> token)
    if ((idx = std::string("qkQK").find(token)) != std::string::npos)
      state_ptr->castling_rights ^= 1 << idx;

  state_ptr->key = side_to_move == WHITE ? 0 : Zobrist::Side;

  for (Square sq = H1; sq <= A8; sq++)
    state_ptr->key ^= Zobrist::hash[piece_on(sq)][sq];

  set_gamephase();

}

std::string Position::to_string() {
  std::stringstream ss;
  ss << "\n+---+---+---+---+---+---+---+---+\n";
  for (Square sq = A8; sq >= H1; sq--) {
    ss << "| " << piece_to_char[board[sq]] << " ";
    if (sq % 8 == 0)
      ss << "| " << (sq / 8 + 1) << "\n+---+---+---+---+---+---+---+---+\n";
  }
  ss << "  a   b   c   d   e   f   g   h\n\nFen: " << fen() << "\nKey: " << std::hex << std::uppercase << key() << "\n";
  return ss.str();
}

std::string Position::fen() {
  std::stringstream fen;
  return "";
}

void Position::do_commit(Move m) {
  if (side_to_move == WHITE)
    do_move<WHITE>(m);
  else
    do_move<BLACK>(m);
  state_stack[0] = *state_ptr;
  state_ptr = state_stack;
  set_gamephase();
  side_to_move = !side_to_move;
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

