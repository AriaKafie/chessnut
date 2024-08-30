
#ifndef DEBUG_H
#define DEBUG_H

#include <sstream>
#include <string>

#include "types.h"

namespace Debug {

void go();
void gameinfo();
void perft(std::istringstream& args);

} // namespace Debug

#endif
