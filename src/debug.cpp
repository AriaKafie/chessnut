
#include "movegen.h"
#include "uci.h"
#include "ui.h"


#include <sstream>
#include <iomanip>
#include <iostream>

namespace Debug {

  void go() {
    std::cout << Position::to_string();
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
    std::cout << std::hex << Position::key() << "\n\n";

  }

}
