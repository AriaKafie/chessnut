
#include "debug.h"
#include "search.h"
#include "board.h"
#include "util.h"
#include "ui.h"
#include "zobrist.h"
#include "gamestate.h"
#include "defs.h"
#include "transpositiontable.h"
#include "uci.h"
#include "bench.h"
#include "evaluation.h"
#include "bitboard.h"
#include "movegen.h"
#include "moveordering.h"
#include "book.h"
#include "magic.h"

#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <ammintrin.h>
#include <regex>

namespace Debug {

  void go() {
    MoveList<WHITE> moves;
    moves.sort(0, 0);
    for (Move m : moves)
      std::cout << Util::move_to_SAN(m) << ": " << std::dec << (m >> 16) << "\n";
  }

  void boardstatus() {

    std::cout << "\n";
    UI::print_board();
    std::cout << "\n\n";
    for (int square = 63; square >= 0; square--) {
      std::string padding = piece_on(square) < 10 ? ",  " : ", ";
      std::cout << piece_on(square) << padding;
      if (square % 8 == 0) std::cout << "\n";
    }
    std::cout << std::hex << Zobrist::key << "\n\n";

  }

}
