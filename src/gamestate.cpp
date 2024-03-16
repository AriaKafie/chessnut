
#include "gamestate.h"
#include "board.h"
#include "util.h"
#include "ui.h"
#include "transpositiontable.h"
#include "search.h"
#include "book.h"

#include <iostream>
#include <algorithm> 

namespace GameState {

  uint8_t castling_rights;

  void update(int move) {
        
    repetition_table.push();
    halfmove_clock++;
    white_to_move = !white_to_move;
    boardstate_save = boardstate;
    update_enpassant(move);
    update_gamephase();

  }

  void restore() {

    repetition_table.pop();
    halfmove_clock--;
    white_to_move = !white_to_move;
    boardstate = boardstate_save;
    update_gamephase();

  }

  void checkmate(bool white_just_moved) {

    if (repetition_table.count_last_hash() > 2) {
      std::cout << "draw by repetition\n";
      std::exit(0);
    }
    if (white_just_moved) {
      MoveList<BLACK> moves;
      if (moves.length() == 0) {
        if (moves.incheck()) {
          if (white_human) std::cout << "human wins\n";
          else std::cout << "computer wins\n";
          std::exit(0);
        }
        std::cout << "stalemate\n";
        std::exit(0);
      }
    }
    else {
      MoveList<WHITE> moves;
      if (moves.length() == 0) {
        if (moves.incheck()) {
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
    std::cout << "]\nep [" << (boardstate & 0x3f) << "]\n\n";

  }

  void parse_fen(std::string fen) {

    for (int i = WHITE; i <= B_KING; i++)
      bitboards[i] = 0;

    for (Square s = H1; s <= A8; s++)
      board[s] = NO_PIECE;

    Square sq = A8;
    size_t idx;

    for (char c : fen) {
      if (std::isdigit(c))
        sq -= c - '0';
      else if (std::isspace(c))
        break;
      else if ((idx = piece_to_char.find(c)) != std::string::npos) {
        Piece p = idx;
        board[sq] = p;
        bitboards[p] ^= square_bb(sq);
        bitboards[color_of(p)] ^= square_bb(sq);
        sq--;
      }
    }

    boardstate = 0;
    castling_rights = 0;
    fen = fen.substr(fen.find(' ') + 1);
    for (int i = 0; i < fen.length(); i++) {
      switch (fen[i]) {
      case ' ':
      case '-':
        continue;
      case 'w':
        white_to_move = true;
        break;
      case 'b':
        white_to_move = false;
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

    Zobrist::update();
    update_gamephase();

  }

  void update_gamephase() {

    int enemy_material = 0;
    int friendly_material = 0;

    if (white_computer) {
      enemy_material += 3 * popcount(bb(B_BISHOP) | bb(B_KNIGHT));
      enemy_material += 5 * popcount(bb(B_ROOK));
      enemy_material += 9 * popcount(bb(B_QUEEN));
      friendly_material += 3 * popcount(bb(W_BISHOP) | bb(W_KNIGHT));
      friendly_material += 5 * popcount(bb(W_ROOK));
      friendly_material += 9 * popcount(bb(W_QUEEN));

      mopup = (enemy_material < 5) && (friendly_material >= 5);
      endgame = (enemy_material < 10) || (enemy_material < 17 && bb(B_QUEEN) == 0);
    }
    else {
      enemy_material += 3 * popcount(bb(W_BISHOP) | bb(W_KNIGHT));
      enemy_material += 5 * popcount(bb(W_ROOK));
      enemy_material += 9 * popcount(bb(W_QUEEN));
      friendly_material += 3 * popcount(bb(B_BISHOP) | bb(B_KNIGHT));
      friendly_material += 5 * popcount(bb(B_ROOK));
      friendly_material += 9 * popcount(bb(B_QUEEN));

      mopup = (enemy_material < 5) && (friendly_material >= 5);
      endgame = (enemy_material < 10) || (enemy_material < 17 && bb(W_QUEEN) == 0);
    }

  }

  void update_enpassant(int move) {

    boardstate &= ~0x3f; // clear enpassant bits
    int from = move & 0x3f;
    int to = (move >> 6) & 0x3f;
    if (type_of(piece_on(to)) == PAWN) {
      if (from - to == 16)
        boardstate += to + 8;
      if (to - from == 16)
        boardstate += to - 8;
    }

  }

}
