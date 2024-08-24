
#include "position.h"

#include <cstring>
#include <random>
#include <sstream>
#include <vector>

#include "transpositiontable.h"
#include "uci.h"
#include "util.h"

void set_gamephase();

Bitboard bitboards[B_KING + 1];
Piece board[SQUARE_NB];

uint64_t Zobrist::hash[B_KING + 1][SQUARE_NB];

void Position::init()
{  
  std::mt19937_64 rng(221564671644);

  for (Piece pc : { W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
                    B_PAWN, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING })
    for (Square sq = H1; sq <= A8; sq++)
      Zobrist::hash[pc][sq] = rng();
}

void Position::set(const std::string& fen)
{
  memset(board, NO_PIECE, 64 * sizeof(Piece));
  memset(bitboards, 0, 16 * sizeof(Bitboard));

  Square             sq = A8;
  std::istringstream is(fen);
  std::string        pieces, color, castling, enpassant;

  is >> pieces >> color >> castling >> enpassant;

  for (char token : pieces)
  {
    if (std::isdigit(token))
      sq -= token - '0';
    else if (size_t piece = piece_to_char.find(token); piece != std::string::npos)
    {
      board[sq] = piece;
      bitboards[piece] ^= square_bb(sq);
      bitboards[color_of(piece)] ^= square_bb(sq);
      sq--;
    }
  }

  side_to_move = color == "w" ? WHITE : BLACK;

  state_ptr->castling_rights = state_ptr->ep_sq = 0;

  for (char token : castling)
    if (size_t idx = std::string("qkQK").find(token); idx != std::string::npos)
      state_ptr->castling_rights ^= 1 << idx;

  if (enpassant != "-")
    state_ptr->ep_sq = uci_to_square(enpassant);

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
  ss << "  a   b   c   d   e   f   g   h\n\nFen: " << fen() << "\nKey: " << std::hex << std::uppercase << key() << "\n\n";
  return ss.str();
}

std::string Position::fen()
{
  std::stringstream fen;

  for (int rank = 7; rank >= 0; rank--)
  {
    for (int file = 7; file >= 0; file--)
    {
      if (Piece pc = piece_on(rank * 8 + file); pc != NO_PIECE)
        fen << piece_to_char[pc];
      else
      {
        int empty_squares = 0, f;
        for (f = file; f >= 0 && !piece_on(rank * 8 + f); f--)
          empty_squares++;
        fen << empty_squares;
        file = f + 1;
      }
    }
    if (rank)
      fen << "/";
  }

  fen << " " << "wb"[side_to_move] << " ";

  if (!state_ptr->castling_rights)
    fen << "-";
  else
  {
    if (kingside_rights <WHITE>()) fen << "K";
    if (queenside_rights<WHITE>()) fen << "Q";
    if (kingside_rights <BLACK>()) fen << "k";
    if (queenside_rights<BLACK>()) fen << "q";
  }                                fen << " " << (state_ptr->ep_sq ? square_to_uci(state_ptr->ep_sq) : "-") << " 0 1";

  return fen.str();
}

void Position::do_commit(Move m)
{
  if (piece_on(to_sq(m)) || piece_type_on(from_sq(m)) == PAWN || type_of(m) != NORMAL)
    RepetitionTable::clear();

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

  Color us = Position::side_to_move, them = !us;

#define piece_count(pc) popcount(bitboards[pc])
  int friendly_material =
    3 * piece_count(make_piece(us, KNIGHT)) +
    3 * piece_count(make_piece(us, BISHOP)) +
    5 * piece_count(make_piece(us, ROOK))   +
    9 * piece_count(make_piece(us, QUEEN));
    
  int enemy_material =
    3 * piece_count(make_piece(them, KNIGHT)) +
    3 * piece_count(make_piece(them, BISHOP)) +
    5 * piece_count(make_piece(them, ROOK))   +
    9 * piece_count(make_piece(them, QUEEN));

  if (enemy_material < 5 && friendly_material >= 5 || friendly_material < 5 && enemy_material >= 5)
    Position::gamephase = MOPUP;
  else if (enemy_material < 10 || enemy_material < 17 && !bitboards[make_piece(them, QUEEN)])
    Position::gamephase = ENDGAME;
  else
    Position::gamephase = MIDGAME;
#undef piece_count

}

