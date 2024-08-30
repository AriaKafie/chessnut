
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

#endif
