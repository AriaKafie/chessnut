
#include "uci.h"
#include "bitboard.h"
#include "util.h"
#include "search.h"

#include <fstream>

int main(int argc, char* argv[]) {

  Bitboards::init();
  Position::init();
  UCI::loop();

  return 0;

}
