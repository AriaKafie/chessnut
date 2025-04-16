
#include "uci.h"

Move Position::uci_to_move(const std::string& uci) const
{
    for (Move list[MAX_MOVES], *m = list, *end = get_moves(list); m != end; m++)
        if (move_to_uci(*m) == uci) return *m;

    return Move::null();
}

std::string move_to_san(Move m, Position& pos)
{
    char pt2c[] = "  PNBRQK";

    Square from = m.from_sq(), to = m.to_sq();
    PieceType pt = pos.piece_type_on(from);
    bool capture = pos.piece_on(to) || m.type_of() == ENPASSANT;

    std::string san;

    if (pt == PAWN)
    {
        if (capture)
            san = std::string(1, "hgfedcba"[from % 8]) + "x" + square_to_uci(to);
        else
            san = square_to_uci(to);

        return m.type_of() == PROMOTION ? san + "=" + pt2c[m.promotion_type()] : san;
    }

    if (capture)
        return std::string(1, pt2c[pt]) + square_to_uci(from) + "x" + square_to_uci(to);

    return std::string(1, pt2c[pt]) + square_to_uci(from) + square_to_uci(to);
}