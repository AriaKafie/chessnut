
#ifndef UTIL_H
#define UTIL_H

#include "movegen.h"
#include "position.h"
#include "uci.h"

#include <bitset>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>
#include <sstream>

inline std::string piece_to_char = "  PNBRQK  pnbrqk";

inline std::string move_to_SAN(Move m) {

  Square      from    = from_sq(m);
  std::string to      = square_to_uci(to_sq(m));
  PieceType   pt      = piece_type_on(from);
  bool        capture = piece_on(to_sq(m)) || type_of(m) == ENPASSANT;

  switch (type_of(m))
  {
    case SHORTCASTLE:
      return "O-O";
    case LONGCASTLE:
      return "O-O-O";
    case NORMAL:
    case ENPASSANT:
      return pt == PAWN ? capture ? std::string(1, char('h' - from % 8)) + "x" + to : to : std::string(1, piece_to_char[pt]) + (capture ? "x" : "") + to;
    case PROMOTION:
      return move_to_SAN(m ^ type_of(m)) + "=Q";
  }
}

inline std::string to_string(Bitboard b) {
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

inline bool is_legal(Move move) {
  if (Position::white_to_move()) {
    MoveList<WHITE> moves;
    for (Move m : moves)
      if (m == move)
        return true;
    return false;
  } else {
    MoveList<BLACK> moves;
    for (Move m : moves)
      if (m == move)
        return true;
    return false;
  }
}

#endif
