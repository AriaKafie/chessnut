
#include "uci.h"
#include "bitboard.h"
#include "util.h"

int main() {

  Bitboards::init();
  Position::init();
  UCI::loop();

  return 0;

}
