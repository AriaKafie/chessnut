
#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "bitboard.h"
#include "movelist.h"
#include "position.h"

inline uint64_t castle_lut[COLOR_NB][1 << 4][1 << 6];

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
    {
        EMove k_castle = make_move<CASTLING>(c == WHITE ? E1 : E8, c == WHITE ? G1 : G8);
        EMove q_castle = make_move<CASTLING>(c == WHITE ? E1 : E8, c == WHITE ? C1 : C8);

        for (int rights = 0; rights <= 0xf; rights++)
        {
            bool k_rights = rights & (c == WHITE ? 8 : 2);
            bool q_rights = rights & (c == WHITE ? 4 : 1);

            for (int hash = 0; hash <= 0x3f; hash++)
            {
                EMove res[2] = {}, *p = res;

                if (k_rights && (hash & 0b000111) == 0)
                    *p++ = k_castle;

                if (q_rights && (hash & 0b111100) == 0)
                    *p++ = q_castle;

                memcpy(&castle_lut[c][rights][hash], res, 8);
            }
        }
    }
}

template<MoveType Type, Direction D>
ForceInline EMove* make_pawn_moves(EMove *list, Bitboard attacks)
{
    for (;attacks; clear_lsb(attacks))
    {
        Square to = bsf(attacks);

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

ForceInline inline EMove* make_moves(EMove *list, Square from, Bitboard to)
{
    for (;to; clear_lsb(to))
        *list++ = make_move(from, bsf(to));

    return list;
}

template<Color Us, Color Them>
MoveList<Us, Them>::MoveList()
{
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
    Bitboard occupied              = Position::occupied();

    seen_by_enemy = pawn_attacks<Them>(bb(OpponentPawn)) | king_attacks(bsf(bb(OpponentKing)));

    for (Bitboard b = bb(OpponentKnight);    b; clear_lsb(b)) seen_by_enemy |= knight_attacks(bsf(b));
    for (Bitboard b = opponent_bishop_queen; b; clear_lsb(b)) seen_by_enemy |= bishop_attacks(bsf(b), occupied ^ bb(FriendlyKing));
    for (Bitboard b = opponent_rook_queen;   b; clear_lsb(b)) seen_by_enemy |= rook_attacks  (bsf(b), occupied ^ bb(FriendlyKing));

    Square ksq = bsf(bb(FriendlyKing));

    checkmask = knight_attacks(ksq) & bb(OpponentKnight) | pawn_attacks<Us>(ksq) & bb(OpponentPawn);

    for (Bitboard checkers = bishop_attacks(ksq, occupied) & opponent_bishop_queen | rook_attacks(ksq, occupied) & opponent_rook_queen; checkers; clear_lsb(checkers))
        checkmask |= check_ray(ksq, bsf(checkers));

    if (more_than_one(checkmask & double_check(ksq)))
    {
        last = make_moves(last, ksq, king_attacks(ksq) & ~(seen_by_enemy | bb(Us)));
        return;
    }

    checkmask |= -!checkmask;

    Bitboard pinned = 0;

    for (Bitboard pinners = bishop_xray(ksq, occupied) & opponent_bishop_queen | rook_xray(ksq, occupied) & opponent_rook_queen; pinners; clear_lsb(pinners))
        pinned |= check_ray(ksq, bsf(pinners));

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
        Square from = bsf(b);
        last = make_moves(last, from, knight_attacks(from) & legal);
    }

    Bitboard bishop_queen = bb(FriendlyBishop) | bb(FriendlyQueen);
    Bitboard rook_queen   = bb(FriendlyRook)   | bb(FriendlyQueen);

    for (Bitboard b = bishop_queen & ~pinned; b; clear_lsb(b))
    {
        Square from = bsf(b);
        last = make_moves(last, from, bishop_attacks(from, occupied) & legal);
    }
    for (Bitboard b = bishop_queen & pinned; b; clear_lsb(b))
    {
        Square from = bsf(b);
        last = make_moves(last, from, bishop_attacks(from, occupied) & legal & align_mask(ksq, from));
    }
    for (Bitboard b = rook_queen & ~pinned; b; clear_lsb(b))
    {
        Square from = bsf(b);
        last = make_moves(last, from, rook_attacks(from, occupied) & legal);
    }
    for (Bitboard b = rook_queen & pinned; b; clear_lsb(b))
    {
        Square from = bsf(b);
        last = make_moves(last, from, rook_attacks(from, occupied) & legal & align_mask(ksq, from));
    }

    last = make_moves(last, ksq, king_attacks(ksq) & ~(seen_by_enemy | bb(Us)));

    constexpr int Shift = Us == WHITE ? 1 : 57;
    constexpr Bitboard NoAtk = Us == WHITE ? square_bb(C1, D1, E1, F1, G1) : square_bb(C8, D8, E8, F8, G8);
    constexpr Bitboard NoOcc = Us == WHITE ? square_bb(B1, C1, D1, F1, G1) : square_bb(B8, C8, D8, F8, G8);

    uint64_t *u = (uint64_t*)last;
    *u = castle_lut[Us][state_ptr->castling_rights][(NoAtk & seen_by_enemy | NoOcc & occupied) >> Shift];
    last += popcount(*u & 0x100000001000);
}

template<Color Us, Color Them>
CaptureList<Us, Them>::CaptureList()
{
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
    Square   ksq                   = bsf(bb(FriendlyKing));
    Bitboard occupied              = Position::occupied() ^ square_bb(ksq);

    seen_by_enemy = pawn_attacks<Them>(bb(OpponentPawn)) | king_attacks(bsf(bb(OpponentKing)));

    for (Bitboard b = bb(OpponentKnight);    b; clear_lsb(b)) seen_by_enemy |= knight_attacks(bsf(b));
    for (Bitboard b = opponent_bishop_queen; b; clear_lsb(b)) seen_by_enemy |= bishop_attacks(bsf(b), occupied);
    for (Bitboard b = opponent_rook_queen;   b; clear_lsb(b)) seen_by_enemy |= rook_attacks  (bsf(b), occupied);

    toggle_square(occupied, ksq);

    Bitboard checkmask = knight_attacks(ksq) & bb(OpponentKnight) | pawn_attacks<Us>(ksq) & bb(OpponentPawn);

    for (Bitboard checkers = bishop_attacks(ksq, occupied) & opponent_bishop_queen | rook_attacks(ksq, occupied) & opponent_rook_queen; checkers; clear_lsb(checkers))
        checkmask |= check_ray(ksq, bsf(checkers));

    if (more_than_one(checkmask & double_check(ksq)))
    {
        last = make_moves(last, ksq, king_attacks(ksq) & bb(Them) & ~seen_by_enemy);
        return;
    }

    checkmask = (checkmask | -!checkmask) & bb(Them);

    Bitboard pinned = 0;

    for (Bitboard pinners = bishop_xray(ksq, occupied) & opponent_bishop_queen | rook_xray(ksq, occupied) & opponent_rook_queen; pinners; clear_lsb(pinners))
        pinned |= check_ray(ksq, bsf(pinners));

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
        Square from = bsf(b);
        last = make_moves(last, from, knight_attacks(from) & checkmask);
    }

    Bitboard bishop_queen = bb(FriendlyBishop) | bb(FriendlyQueen);
    Bitboard rook_queen   = bb(FriendlyRook)   | bb(FriendlyQueen);

    for (Bitboard b = bishop_queen & ~pinned; b; clear_lsb(b))
    {
        Square from = bsf(b);
        last = make_moves(last, from, bishop_attacks(from, occupied) & checkmask);
    }
    for (Bitboard b = bishop_queen & pinned; b; clear_lsb(b))
    {
        Square from = bsf(b);
        last = make_moves(last, from, bishop_attacks(from, occupied) & checkmask & align_mask(ksq, from));
    }
    for (Bitboard b = rook_queen & ~pinned; b; clear_lsb(b))
    {
        Square from = bsf(b);
        last = make_moves(last, from, rook_attacks(from, occupied) & checkmask);
    }
    for (Bitboard b = rook_queen & pinned; b; clear_lsb(b))
    {
        Square from = bsf(b);
        last = make_moves(last, from, rook_attacks(from, occupied) & checkmask & align_mask(ksq, from));
    }

    last = make_moves(last, ksq, king_attacks(ksq) & bb(Them) & ~seen_by_enemy);
}

#endif
