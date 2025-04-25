
#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "bitboard.h"
#include "movelist.h"
#include "position.h"

inline constexpr Move data_[COLOR_NB][5] =
{
    { make_move<CASTLING>(E1, G1), make_move<CASTLING>(E1, C1), NULLMOVE, make_move<CASTLING>(E1, G1), NULLMOVE },
    { make_move<CASTLING>(E8, G8), make_move<CASTLING>(E8, C8), NULLMOVE, make_move<CASTLING>(E8, G8), NULLMOVE }
};

inline const Move *table_[COLOR_NB][1 << 4][1 << 6];

namespace MoveGen { void init(); };
/*
+---+---+---+---+---+---+---+---+
|   |qo |qoa|qoa|qka|koa|koa|   | >> 1,
+---+---+---+---+---+---+---+---+
|   |   |qo |qoa|qoa|qka|koa|koa|
+---+---+---+---+---+---+---+---+
*/
inline void MoveGen::init()
{
    for (Color c : { WHITE, BLACK })
        for (int rights = 0; rights <= 0xf; rights++)
            for (Bitboard hash = 0; hash <= 0b111111; hash++)
            {
                const Move *kcastle   = &data_[c][3];
                const Move *qcastle   = &data_[c][1];
                const Move *both      = &data_[c][0];
                const Move *no_castle = &data_[c][2];
                
                const Move *src = no_castle;

                bool rights_k = rights & (c == WHITE ? 8 : 2);
                bool rights_q = rights & (c == WHITE ? 4 : 1);
                bool rights_kq = rights_k && rights_q;

                if (rights_k || rights_q)
                {
                    if      (hash == 0)              src = rights_kq ? both : rights_k ? kcastle : qcastle;
                    else if ((hash & 0b000111) == 0) src = rights_k ? kcastle : no_castle;
                    else if ((hash & 0b111100) == 0) src = rights_q ? qcastle : no_castle;
                }

                table_[c][rights][hash] = src;
            }
}

template<MoveType Type, Direction D>
ForceInline LMove *make_pawn_moves(LMove *list, Bitboard attacks)
{
    for (;attacks; clear_lsb(attacks))
    {
        Square to = lsb(attacks);

        if constexpr (Type == NORMAL)
        {
            *list++ = make_move(to - D, to);
        }
        else if constexpr (Type == PROMOTION)
        {
            *list++ = make_move<KNIGHT_PROMOTION>(to - D, to);
            *list++ = make_move<BISHOP_PROMOTION>(to - D, to);
            *list++ = make_move<ROOK_PROMOTION  >(to - D, to);
            *list++ = make_move<QUEEN_PROMOTION >(to - D, to);
        }
    }

    return list;
}

ForceInline inline LMove *make_moves(LMove *list, Square from, Bitboard to)
{
    for (;to; clear_lsb(to))
        *list++ = make_move(from, lsb(to));

    return list;
}

template<Color Us>
MoveList<Us>::MoveList()
{
    constexpr Color Them           = !Us;
    constexpr Piece FriendlyPawn   = make_piece(Us, PAWN);
    constexpr Piece FriendlyKnight = make_piece(Us, KNIGHT);
    constexpr Piece FriendlyBishop = make_piece(Us, BISHOP);
    constexpr Piece FriendlyRook   = make_piece(Us, ROOK);
    constexpr Piece FriendlyQueen  = make_piece(Us, QUEEN);
    constexpr Piece FriendlyKing   = make_piece(Us, KING);
    constexpr Piece OpponentPawn   = make_piece(Them, PAWN);
    constexpr Piece OpponentKnight = make_piece(Them, KNIGHT);
    constexpr Piece OpponentBishop = make_piece(Them, BISHOP);
    constexpr Piece OpponentRook   = make_piece(Them, ROOK);
    constexpr Piece OpponentQueen  = make_piece(Them, QUEEN);
    constexpr Piece OpponentKing   = make_piece(Them, KING);

    Bitboard opponent_rook_queen   = bb(OpponentQueen) | bb(OpponentRook);
    Bitboard opponent_bishop_queen = bb(OpponentQueen) | bb(OpponentBishop);
    Square   ksq                   = lsb(bb(FriendlyKing));
    Bitboard occupied              = Position::occupied() ^ square_bb(ksq);

    seen_by_enemy = pawn_attacks<Them>(bb(OpponentPawn)) | king_attacks(lsb(bb(OpponentKing)));

    for (Bitboard b = bb(OpponentKnight);    b; clear_lsb(b)) seen_by_enemy |= knight_attacks(lsb(b));
    for (Bitboard b = opponent_bishop_queen; b; clear_lsb(b)) seen_by_enemy |= bishop_attacks(lsb(b), occupied);
    for (Bitboard b = opponent_rook_queen;   b; clear_lsb(b)) seen_by_enemy |= rook_attacks  (lsb(b), occupied);

    toggle_square(occupied, ksq);

    checkmask = knight_attacks(ksq) & bb(OpponentKnight) | pawn_attacks<Us>(ksq) & bb(OpponentPawn);

    for (Bitboard checkers = bishop_attacks(ksq, occupied) & opponent_bishop_queen | rook_attacks(ksq, occupied) & opponent_rook_queen; checkers; clear_lsb(checkers))
        checkmask |= check_ray(ksq, lsb(checkers));

    if (more_than_one(checkmask & double_check(ksq)))
    {
        last = make_moves(last, ksq, king_attacks(ksq) & ~(seen_by_enemy | bb(Us)));
        return;
    }

    checkmask |= -!checkmask;

    Bitboard pinned = 0;

    for (Bitboard pinners = bishop_xray(ksq, occupied) & opponent_bishop_queen | rook_xray(ksq, occupied) & opponent_rook_queen; pinners; clear_lsb(pinners))
        pinned |= check_ray(ksq, lsb(pinners));

    constexpr Direction Up      = Us == WHITE ? NORTH      : SOUTH;
    constexpr Direction UpRight = Us == WHITE ? NORTH_EAST : SOUTH_WEST;
    constexpr Direction UpLeft  = Us == WHITE ? NORTH_WEST : SOUTH_EAST;
    constexpr Bitboard  Rank2   = Us == WHITE ? RANK_2BB   : RANK_7BB;
    constexpr Bitboard  Rank3   = Us == WHITE ? RANK_3BB   : RANK_6BB;
    constexpr Bitboard  Rank6   = Us == WHITE ? RANK_6BB   : RANK_3BB;
    constexpr Bitboard  Rank7   = Us == WHITE ? RANK_7BB   : RANK_2BB;

    Bitboard empty = ~occupied;
    Bitboard e     = shift<Up>(Rank3 & empty) & empty;
    Bitboard pawns = bb(FriendlyPawn) & ~Rank7;

    last = make_pawn_moves<NORMAL, UpRight>(last, shift<UpRight>(pawns & (~pinned | anti_diag(ksq))) & bb(Them) & checkmask);
    last = make_pawn_moves<NORMAL, UpLeft >(last, shift<UpLeft >(pawns & (~pinned | main_diag(ksq))) & bb(Them) & checkmask);
    last = make_pawn_moves<NORMAL, Up     >(last, shift<Up     >(pawns & (~pinned | file_bb  (ksq))) & empty    & checkmask);
    last = make_pawn_moves<NORMAL, Up*2   >(last, shift<Up*2   >(pawns & (~pinned | file_bb  (ksq))) & e        & checkmask);

    if (Bitboard promotable = bb(FriendlyPawn) & Rank7)
    {
        last = make_pawn_moves<PROMOTION, UpRight>(last, shift<UpRight>(promotable & (~pinned | anti_diag(ksq))) & bb(Them) & checkmask);
        last = make_pawn_moves<PROMOTION, UpLeft >(last, shift<UpLeft >(promotable & (~pinned | main_diag(ksq))) & bb(Them) & checkmask);
        last = make_pawn_moves<PROMOTION, Up     >(last, shift<Up     >(promotable &  ~pinned                  ) & empty    & checkmask);
    }

    if (Bitboard b = shift<UpRight>(bb(FriendlyPawn)) & Position::ep_bb() & Rank6)
    {
        *last = make_move<ENPASSANT>(state_ptr->ep_sq - UpRight, state_ptr->ep_sq);
        Bitboard after_ep = occupied ^ (b | shift_unsafe<-UpRight>(b) | shift_unsafe<-Up>(b));
        last += !(bishop_attacks(ksq, after_ep) & opponent_bishop_queen | rook_attacks(ksq, after_ep) & opponent_rook_queen);
    }
    if (Bitboard b = shift<UpLeft >(bb(FriendlyPawn)) & Position::ep_bb() & Rank6)
    {
        *last = make_move<ENPASSANT>(state_ptr->ep_sq - UpLeft, state_ptr->ep_sq);
        Bitboard after_ep = occupied ^ (b | shift_unsafe<-UpLeft>(b) | shift_unsafe<-Up>(b));
        last += !(bishop_attacks(ksq, after_ep) & opponent_bishop_queen | rook_attacks(ksq, after_ep) & opponent_rook_queen);
    }

    Bitboard legal = checkmask &~ bb(Us);

    for (Bitboard b = bb(FriendlyKnight) & ~pinned; b; clear_lsb(b))
    {
        Square from = lsb(b);
        last = make_moves(last, from, knight_attacks(from) & legal);
    }

    Bitboard bishop_queen = bb(FriendlyBishop) | bb(FriendlyQueen);
    Bitboard rook_queen   = bb(FriendlyRook)   | bb(FriendlyQueen);

    for (Bitboard b = bishop_queen & ~pinned; b; clear_lsb(b))
    {
        Square from = lsb(b);
        last = make_moves(last, from, bishop_attacks(from, occupied) & legal);
    }
    for (Bitboard b = bishop_queen & pinned; b; clear_lsb(b))
    {
        Square from = lsb(b);
        last = make_moves(last, from, bishop_attacks(from, occupied) & legal & align_mask(ksq, from));
    }
    for (Bitboard b = rook_queen & ~pinned; b; clear_lsb(b))
    {
        Square from = lsb(b);
        last = make_moves(last, from, rook_attacks(from, occupied) & legal);
    }
    for (Bitboard b = rook_queen & pinned; b; clear_lsb(b))
    {
        Square from = lsb(b);
        last = make_moves(last, from, rook_attacks(from, occupied) & legal & align_mask(ksq, from));
    }

    last = make_moves(last, ksq, king_attacks(ksq) & ~(seen_by_enemy | bb(Us)));

    constexpr int Shift = Us == WHITE ? 1 : 57;

    constexpr Bitboard NoAtk = Us == WHITE ? square_bb(C1, D1, E1, F1, G1) : square_bb(C8, D8, E8, F8, G8);
    constexpr Bitboard NoOcc = Us == WHITE ? square_bb(B1, C1, D1, F1, G1) : square_bb(B8, C8, D8, F8, G8);

    for (const Move *src = table_[Us][state_ptr->castling_rights][(NoAtk & seen_by_enemy | NoOcc & occupied) >> Shift]; *src; *last++ = *src++);
}

template<Color Us>
CaptureList<Us>::CaptureList()
{
    constexpr Color Them           = !Us;
    constexpr Piece FriendlyPawn   = make_piece(Us, PAWN);
    constexpr Piece FriendlyKnight = make_piece(Us, KNIGHT);
    constexpr Piece FriendlyBishop = make_piece(Us, BISHOP);
    constexpr Piece FriendlyRook   = make_piece(Us, ROOK);
    constexpr Piece FriendlyQueen  = make_piece(Us, QUEEN);
    constexpr Piece FriendlyKing   = make_piece(Us, KING);
    constexpr Piece OpponentPawn   = make_piece(Them, PAWN);
    constexpr Piece OpponentKnight = make_piece(Them, KNIGHT);
    constexpr Piece OpponentBishop = make_piece(Them, BISHOP);
    constexpr Piece OpponentRook   = make_piece(Them, ROOK);
    constexpr Piece OpponentQueen  = make_piece(Them, QUEEN);
    constexpr Piece OpponentKing   = make_piece(Them, KING);

    Bitboard opponent_rook_queen   = bb(OpponentQueen) | bb(OpponentRook);
    Bitboard opponent_bishop_queen = bb(OpponentQueen) | bb(OpponentBishop);
    Square   ksq                   = lsb(bb(FriendlyKing));
    Bitboard occupied              = Position::occupied() ^ square_bb(ksq);

    seen_by_enemy = pawn_attacks<Them>(bb(OpponentPawn)) | king_attacks(lsb(bb(OpponentKing)));

    for (Bitboard b = bb(OpponentKnight);    b; clear_lsb(b)) seen_by_enemy |= knight_attacks(lsb(b));
    for (Bitboard b = opponent_bishop_queen; b; clear_lsb(b)) seen_by_enemy |= bishop_attacks(lsb(b), occupied);
    for (Bitboard b = opponent_rook_queen;   b; clear_lsb(b)) seen_by_enemy |= rook_attacks  (lsb(b), occupied);

    toggle_square(occupied, ksq);

    Bitboard checkmask = knight_attacks(ksq) & bb(OpponentKnight) | pawn_attacks<Us>(ksq) & bb(OpponentPawn);

    for (Bitboard checkers = bishop_attacks(ksq, occupied) & opponent_bishop_queen | rook_attacks(ksq, occupied) & opponent_rook_queen; checkers; clear_lsb(checkers))
        checkmask |= check_ray(ksq, lsb(checkers));

    if (more_than_one(checkmask & double_check(ksq)))
    {
        last = make_moves(last, ksq, king_attacks(ksq) & bb(Them) & ~seen_by_enemy);
        return;
    }

    checkmask = (checkmask | -!checkmask) & bb(Them);

    Bitboard pinned = 0;

    for (Bitboard pinners = bishop_xray(ksq, occupied) & opponent_bishop_queen | rook_xray(ksq, occupied) & opponent_rook_queen; pinners; clear_lsb(pinners))
        pinned |= check_ray(ksq, lsb(pinners));

    constexpr Direction UpRight = Us == WHITE ? NORTH_EAST : SOUTH_WEST;
    constexpr Direction UpLeft  = Us == WHITE ? NORTH_WEST : SOUTH_EAST;
    constexpr Bitboard  Rank7   = Us == WHITE ? RANK_7BB   : RANK_2BB;

    if (Bitboard promotable = bb(FriendlyPawn) & Rank7)
    {
        last = make_pawn_moves<PROMOTION, UpRight>(last, shift<UpRight>(promotable & (~pinned | anti_diag(ksq))) & checkmask);
        last = make_pawn_moves<PROMOTION, UpLeft >(last, shift<UpLeft >(promotable & (~pinned | main_diag(ksq))) & checkmask);
    }

    Bitboard pawns = bb(FriendlyPawn) & ~Rank7;

    last = make_pawn_moves<NORMAL, UpRight>(last, shift<UpRight>(pawns & (~pinned | anti_diag(ksq))) & checkmask);
    last = make_pawn_moves<NORMAL, UpLeft >(last, shift<UpLeft >(pawns & (~pinned | main_diag(ksq))) & checkmask);

    for (Bitboard b = bb(FriendlyKnight) & ~pinned; b; clear_lsb(b))
    {
        Square from = lsb(b);
        last = make_moves(last, from, knight_attacks(from) & checkmask);
    }

    Bitboard bishop_queen = bb(FriendlyBishop) | bb(FriendlyQueen);
    Bitboard rook_queen   = bb(FriendlyRook)   | bb(FriendlyQueen);

    for (Bitboard b = bishop_queen & ~pinned; b; clear_lsb(b))
    {
        Square from = lsb(b);
        last = make_moves(last, from, bishop_attacks(from, occupied) & checkmask);
    }
    for (Bitboard b = bishop_queen & pinned; b; clear_lsb(b))
    {
        Square from = lsb(b);
        last = make_moves(last, from, bishop_attacks(from, occupied) & checkmask & align_mask(ksq, from));
    }
    for (Bitboard b = rook_queen & ~pinned; b; clear_lsb(b))
    {
        Square from = lsb(b);
        last = make_moves(last, from, rook_attacks(from, occupied) & checkmask);
    }
    for (Bitboard b = rook_queen & pinned; b; clear_lsb(b))
    {
        Square from = lsb(b);
        last = make_moves(last, from, rook_attacks(from, occupied) & checkmask & align_mask(ksq, from));
    }

    last = make_moves(last, ksq, king_attacks(ksq) & bb(Them) & ~seen_by_enemy);
}

#endif
