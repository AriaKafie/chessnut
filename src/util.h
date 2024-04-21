
#ifndef UTIL_H
#define UTIL_H

#include "position.h"

#include "ui.h"
#include "movegen.h"

#include <bitset>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>
#include <sstream>

inline std::string piece_to_char = "  PNBRQK  pnbrqk";

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

inline std::string bbtos(Bitboard b) {
  std::string l, s;
  l = s = "+---+---+---+---+---+---+---+---+\n";
  for (Bitboard bit = square_bb(A8); bit; bit >>= 1) {
    s += (bit & b) ? "| @ " : "|   ";
    if (bit & FILE_H)
      s += "|\n" + l;
  }
  return s + "\n";
}

inline unsigned long long curr_time_millis() {
  return std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();
}

namespace Util {

  inline bool starts_with(const std::string& str, const std::string& prefix) {
    return str.compare(0, prefix.length(), prefix) == 0;
  }

  inline int SAN_to_int(std::string sanstr) {
    if (Position::white_to_move()) {
      MoveList<WHITE> ml;
      for (Move m : ml) {
        if (move_to_SAN(m) == sanstr)
          return m;
      }
      return NULLMOVE;
    }
    else {
      MoveList<BLACK> ml;
      for (Move m : ml) {
        if (move_to_SAN(m) == sanstr)
          return m;
      }
      return NULLMOVE;
    }
  }

  inline void print_binary(uint8_t num) {
        
    std::bitset<sizeof(uint8_t) * 8> binary(num);
    std::cout << binary << std::endl;

  }

}

#endif
