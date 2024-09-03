
#ifndef UCI_H
#define UCI_H

#include <string>

#include "types.h"

Move uci_to_move(const std::string& uci);

namespace UCI { void loop(); }

inline std::string square_to_uci(Square sq) {
    return std::string(1, "hgfedcba"[sq % 8]) + std::string(1, "12345678"[sq / 8]);
}

inline std::string move_to_uci(Move m) {
    return square_to_uci(from_sq(m)) + square_to_uci(to_sq(m)) + (type_of(m) == PROMOTION ? "q" : "");
}

inline Square uci_to_square(const std::string& uci) {
    return 8 * (uci[1] - '1') + 'h' - uci[0];
}

#endif
