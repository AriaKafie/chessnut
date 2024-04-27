
#include "uci.h"
#include "bitboard.h"
#include "util.h"
#include "search.h"

int main(int argc, char* argv[]) {

  Bitboards::init();
  Search::init();
  Position::init();
  UCI::loop();

  return 0;

}
