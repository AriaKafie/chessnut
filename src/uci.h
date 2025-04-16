
#ifndef UCI_H
#define UCI_H

#include <string>

#include "position.h"
#include "types.h"

inline Square uci_to_square(const std::string& uci) {
    return 8 * (uci[1] - '1') + 'h' - uci[0];
}

inline std::string square_to_uci(Square sq) {
    return std::string(1, "hgfedcba"[sq % 8]) + std::string(1, "12345678"[sq / 8]);
}

inline std::string move_to_uci(Move m)
{
    return m.type_of() == PROMOTION ? square_to_uci(m.from_sq()) + square_to_uci(m.to_sq()) + "   nbrq"[m.promotion_type()]
                                    : square_to_uci(m.from_sq()) + square_to_uci(m.to_sq());
}

std::string move_to_san(Move m, Position& pos);

#endif