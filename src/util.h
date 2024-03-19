
#ifndef UTIL_H
#define UTIL_H

#include "board.h"
#include "defs.h"
#include "ui.h"
#include "gamestate.h"
#include "movegen.h"

#include <bitset>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>

inline std::string piece_to_char = "  PNBRQK  pnbrqk";

namespace Util {

  inline bool starts_with(const std::string& str, const std::string& prefix) {
    return str.compare(0, prefix.length(), prefix) == 0;
  }

  inline std::string move_to_SAN(Move m) {

    Square    from    = from_sq(m);
    Square    to      = to_sq(m);
    PieceType pt      = piece_type_on(from);
    bool      capture = piece_on(to) || type_of(m) == ENPASSANT;

    switch (type_of(m))
    {
      case SHORTCASTLE:
        return "O-O";
      case LONGCASTLE:
        return "O-O-O";
      case NORMAL:
      case ENPASSANT:
        return pt == PAWN ? capture ? std::string(1, char('h' - from % 8)) + "x" + UI::coords[to] : UI::coords[to] : std::string(1, piece_to_char[pt]) + (capture ? "x" : "") + UI::coords[to];
      case PROMOTION:
        return move_to_SAN(m ^ type_of(m)) + "=Q";
    }
  }

  inline int SAN_to_int(std::string sanstr) {
    if (GameState::white_to_move) {
      MoveList<WHITE> ml(true);
      for (Move m : ml) {
        if (move_to_SAN(m) == sanstr)
          return m;
      }
      return NULLMOVE;
    }
    else {
      MoveList<BLACK> ml(true);
      for (Move m : ml) {
        if (move_to_SAN(m) == sanstr)
          return m;
      }
      return NULLMOVE;
    }
  }

  inline std::string get_fen() {

    std::string fen = "";

    for (int i = 63; i >= 0; i--) {
      if ((i % 8 == 7) && (i != 63)) fen += "/";
      int square_type = piece_on(i);
      if (square_type == NO_PIECE) {
        bool break_twice = false;
        int num_empty_squares = 0;
        for (int j = i; piece_on(j) == NO_PIECE; j--) {
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
      fen += piece_to_char[square_type];
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

  unsigned long long curr_time_millis();
  uint64_t string_to_long(std::string str);
  std::string long_to_string(uint64_t num);
  void print_bitboard_and_num(uint64_t bb);

  inline void print_bitboard_and_num(uint64_t bb) {

    std::string s = "+---+---+---+---+---+---+---+---+";
    for (int i = 0; i < 64; i++) {
      if (i % 8 == 0)
        std::cout << "|\n" << s << "\n";
      std::cout << "| " << ((bb & (1ull << (i ^ 63))) ? "@" : " ") << " ";
    }
    std::cout << "|\n" << s << "\n" << "0x" << std::hex << bb << "ull\n";

  }

  inline unsigned long long curr_time_millis() {
    return std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();
  }

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

#endif
