
#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "bitboard.h"
#include "movelist.h"
#include "position.h"

ForceInline inline Move* make_moves(Move* list, Square from, Bitboard to) {

    for (;to; pop_lsb(to))
        *list++ = make_move(from, lsb(to));

    return list;
}

template<Direction D>
ForceInline Move* make_pawn_moves(Move* list, Bitboard attacks) {

    for (;attacks; pop_lsb(attacks))
    {
        Square to = lsb(attacks);
        *list++ = make_move(to - D, to);
    }

    return list;
}

template<Color Us>
MoveList<Us>::MoveList()
{
    constexpr Color Them           = !Us;
    constexpr Piece FriendlyPawn   = make_piece(Us,   PAWN);
    constexpr Piece EnemyPawn      = make_piece(Them, PAWN);
    constexpr Piece FriendlyKnight = make_piece(Us,   KNIGHT);
    constexpr Piece EnemyKnight    = make_piece(Them, KNIGHT);
    constexpr Piece FriendlyBishop = make_piece(Us,   BISHOP);
    constexpr Piece EnemyBishop    = make_piece(Them, BISHOP);
    constexpr Piece FriendlyRook   = make_piece(Us,   ROOK);
    constexpr Piece EnemyRook      = make_piece(Them, ROOK);
    constexpr Piece FriendlyQueen  = make_piece(Us,   QUEEN);
    constexpr Piece EnemyQueen     = make_piece(Them, QUEEN);
    constexpr Piece FriendlyKing   = make_piece(Us,   KING);
    constexpr Piece EnemyKing      = make_piece(Them, KING);

    Bitboard enemy_rook_queen   = bb(EnemyQueen) | bb(EnemyRook);
    Bitboard enemy_bishop_queen = bb(EnemyQueen) | bb(EnemyBishop);
    Square   ksq                = lsb(bb(FriendlyKing));
    Bitboard occupied           = occupied_bb() ^ square_bb(ksq);

    seen_by_enemy = pawn_attacks<Them>(bb(EnemyPawn)) | king_attacks(lsb(bb(EnemyKing)));

    for (Bitboard b = bb(EnemyKnight);    b; pop_lsb(b)) seen_by_enemy |= knight_attacks(lsb(b));
    for (Bitboard b = enemy_bishop_queen; b; pop_lsb(b)) seen_by_enemy |= bishop_attacks(lsb(b), occupied);
    for (Bitboard b = enemy_rook_queen;   b; pop_lsb(b)) seen_by_enemy |= rook_attacks  (lsb(b), occupied);

    toggle_square(occupied, ksq);

    checkmask = knight_attacks(ksq) & bb(EnemyKnight) | pawn_attacks<Us>(ksq) & bb(EnemyPawn);

    for (Bitboard checkers = bishop_attacks(ksq, occupied) & enemy_bishop_queen | rook_attacks(ksq, occupied) & enemy_rook_queen; checkers; pop_lsb(checkers))
        checkmask |= check_ray(ksq, lsb(checkers));

    if (more_than_one(checkmask & double_check(ksq)))
    {
        last = make_moves(last, ksq, king_attacks(ksq) & ~(seen_by_enemy | bb(Us)));
        return;
    }

    checkmask |= -!checkmask;

    Bitboard pinned = 0;

    for (Bitboard pinners = bishop_xray(ksq, occupied) & enemy_bishop_queen | rook_xray(ksq, occupied) & enemy_rook_queen; pinners; pop_lsb(pinners))
        pinned |= check_ray(ksq, lsb(pinners));

    constexpr Direction Up        = Us == WHITE ? NORTH      : SOUTH;
    constexpr Direction Up2       = Us == WHITE ? NORTH * 2  : SOUTH * 2;
    constexpr Direction UpRight   = Us == WHITE ? NORTH_EAST : SOUTH_WEST;
    constexpr Direction UpLeft    = Us == WHITE ? NORTH_WEST : SOUTH_EAST;
    constexpr Bitboard  FriendEP  = Us == WHITE ? RANK_3     : RANK_6;
    constexpr Bitboard  EnemyEP   = Us == WHITE ? RANK_6     : RANK_3;
    constexpr Bitboard  Start     = Us == WHITE ? RANK_2     : RANK_7;
    constexpr Bitboard  Promote   = Us == WHITE ? RANK_7     : RANK_2;
    constexpr Bitboard  NoPromote = ~Promote;

    Bitboard empty      = ~occupied;
    Bitboard not_pinned = ~pinned;
    Bitboard pawns      = bb(FriendlyPawn) & NoPromote;
    Bitboard e          = shift<Up>(FriendEP & empty) & empty;

    last = make_pawn_moves<UpRight>(last, shift<UpRight>(pawns & (not_pinned | anti_diag(ksq))) & bb(Them) & checkmask);
    last = make_pawn_moves<UpLeft >(last, shift<UpLeft >(pawns & (not_pinned | main_diag(ksq))) & bb(Them) & checkmask);
    last = make_pawn_moves<Up     >(last, shift<Up     >(pawns & (not_pinned | file_bb  (ksq))) & empty    & checkmask);
    last = make_pawn_moves<Up2    >(last, shift<Up2    >(pawns & (not_pinned | file_bb  (ksq))) & e        & checkmask);

    if (Bitboard promotable = bb(FriendlyPawn) & Promote)
    {
        for (Bitboard b = shift<UpRight>(promotable & (not_pinned | anti_diag(ksq))) & bb(Them) & checkmask; b; pop_lsb(b))
        {
            Square to = lsb(b);
            *last++ = make_move<PROMOTION>(to - UpRight, to);
        }
        for (Bitboard b = shift<UpLeft >(promotable & (not_pinned | main_diag(ksq))) & bb(Them) & checkmask; b; pop_lsb(b))
        {
            Square to = lsb(b);
            *last++ = make_move<PROMOTION>(to - UpLeft, to);
        }
        for (Bitboard b = shift<Up>(promotable & not_pinned) & empty & checkmask; b; pop_lsb(b))
        {
            Square to = lsb(b);
            *last++ = make_move<PROMOTION>(to - Up, to);
        }
    }

    if (Bitboard b = shift<UpRight>(bb(FriendlyPawn)) & Position::ep_bb() & EnemyEP)
    {
        Square to = lsb(b);
        *last = make_move<ENPASSANT>(to - UpRight, to);
        Bitboard ep_toggle = b | shift<-UpRight>(b) | shift<-Up>(b);
        Bitboard o = occupied ^ ep_toggle;
        last += !(bishop_attacks(ksq, o) & enemy_bishop_queen | rook_attacks(ksq, o) & enemy_rook_queen);
    }
    if (Bitboard b = shift<UpLeft >(bb(FriendlyPawn)) & Position::ep_bb() & EnemyEP)
    {
        Square to = lsb(b);
        *last = make_move<ENPASSANT>(to - UpLeft, to);
        Bitboard ep_toggle = b | shift<-UpLeft>(b) | shift<-Up>(b);
        Bitboard o = occupied ^ ep_toggle;
        last += !(bishop_attacks(ksq, o) & enemy_bishop_queen | rook_attacks(ksq, o) & enemy_rook_queen);
    }

    Bitboard friendly_rook_queen   = bb(FriendlyQueen) | bb(FriendlyRook);
    Bitboard friendly_bishop_queen = bb(FriendlyQueen) | bb(FriendlyBishop);

    Bitboard legal = checkmask &~ bb(Us);

    for (Bitboard b = bb(FriendlyKnight) & not_pinned; b; pop_lsb(b))
    {
        Square from = lsb(b);
        last = make_moves(last, from, knight_attacks(from) & legal);
    }
    for (Bitboard b = friendly_bishop_queen & not_pinned; b; pop_lsb(b))
    {
        Square from = lsb(b);
        last = make_moves(last, from, bishop_attacks(from, occupied) & legal);
    }
    for (Bitboard b = friendly_bishop_queen & pinned; b; pop_lsb(b))
    {
        Square from = lsb(b);
        last = make_moves(last, from, bishop_attacks(from, occupied) & legal & pin_mask(ksq, from));
    }
    for (Bitboard b = friendly_rook_queen & not_pinned; b; pop_lsb(b))
    {
        Square from = lsb(b);
        last = make_moves(last, from, rook_attacks(from, occupied) & legal);
    }
    for (Bitboard b = friendly_rook_queen & pinned; b; pop_lsb(b))
    {
        Square from = lsb(b);
        last = make_moves(last, from, rook_attacks(from, occupied) & legal & pin_mask(ksq, from));
    }

    last = make_moves(last, ksq, king_attacks(ksq) & ~(seen_by_enemy | bb(Us)));

    constexpr Bitboard k_no_atk = Us == WHITE ? square_bb(E1, F1, G1) : square_bb(E8, F8, G8);
    constexpr Bitboard k_no_occ = Us == WHITE ? square_bb(F1, G1)     : square_bb(F8, G8);
    constexpr Bitboard q_no_atk = Us == WHITE ? square_bb(C1, D1, E1) : square_bb(C8, D8, E8);
    constexpr Bitboard q_no_occ = Us == WHITE ? square_bb(B1, C1, D1) : square_bb(B8, C8, D8);

    constexpr Move SCASTLE = Us == WHITE ? make_move<SHORTCASTLE>(E1, G1) : make_move<SHORTCASTLE>(E8, G8);
    constexpr Move LCASTLE = Us == WHITE ? make_move<LONGCASTLE >(E1, C1) : make_move<LONGCASTLE >(E8, C8);

    *last = SCASTLE;
    last += !((seen_by_enemy & k_no_atk | occupied & k_no_occ | !Position::kingside_rights <Us>()));

    *last = LCASTLE;
    last += !((seen_by_enemy & q_no_atk | occupied & q_no_occ | !Position::queenside_rights<Us>()));
}

template<Color Us>
CaptureList<Us>::CaptureList() :
  last(moves)
{
    constexpr Color Them           = !Us;
    constexpr Piece FriendlyPawn   = make_piece(Us,   PAWN);
    constexpr Piece EnemyPawn      = make_piece(Them, PAWN);
    constexpr Piece FriendlyKnight = make_piece(Us,   KNIGHT);
    constexpr Piece EnemyKnight    = make_piece(Them, KNIGHT);
    constexpr Piece FriendlyBishop = make_piece(Us,   BISHOP);
    constexpr Piece EnemyBishop    = make_piece(Them, BISHOP);
    constexpr Piece FriendlyRook   = make_piece(Us,   ROOK);
    constexpr Piece EnemyRook      = make_piece(Them, ROOK);
    constexpr Piece FriendlyQueen  = make_piece(Us,   QUEEN);
    constexpr Piece EnemyQueen     = make_piece(Them, QUEEN);
    constexpr Piece FriendlyKing   = make_piece(Us,   KING);
    constexpr Piece EnemyKing      = make_piece(Them, KING);

    Bitboard enemy_rook_queen   = bb(EnemyQueen) | bb(EnemyRook);
    Bitboard enemy_bishop_queen = bb(EnemyQueen) | bb(EnemyBishop);
    Square   ksq                = lsb(bb(FriendlyKing));
    Bitboard occupied           = occupied_bb() ^ square_bb(ksq);

    seen_by_enemy = pawn_attacks<Them>(bb(EnemyPawn)) | king_attacks(lsb(bb(EnemyKing)));

    for (Bitboard b = bb(EnemyKnight);    b; pop_lsb(b)) seen_by_enemy |= knight_attacks(lsb(b));
    for (Bitboard b = enemy_bishop_queen; b; pop_lsb(b)) seen_by_enemy |= bishop_attacks(lsb(b), occupied);
    for (Bitboard b = enemy_rook_queen;   b; pop_lsb(b)) seen_by_enemy |= rook_attacks  (lsb(b), occupied);

    toggle_square(occupied, ksq);

    Bitboard enemy_unprotected = bb(Them) &~ seen_by_enemy;

    checkmask = knight_attacks(ksq) & bb(EnemyKnight) | pawn_attacks<Us>(ksq) & bb(EnemyPawn);

    for (Bitboard checkers = bishop_attacks(ksq, occupied) & enemy_bishop_queen | rook_attacks(ksq, occupied) & enemy_rook_queen; checkers; pop_lsb(checkers))
        checkmask |= check_ray(ksq, lsb(checkers));

    if (more_than_one(checkmask & double_check(ksq)))
    {
        last = make_moves(last, ksq, king_attacks(ksq) & enemy_unprotected);
        return;
    }

    checkmask |= -!checkmask;

    checkmask &= bb(Them);

    Bitboard pinned = 0;

    for (Bitboard pinners = bishop_xray(ksq, occupied) & enemy_bishop_queen | rook_xray(ksq, occupied) & enemy_rook_queen; pinners; pop_lsb(pinners))
        pinned |= check_ray(ksq, lsb(pinners));

    constexpr Direction UpRight   = Us == WHITE ? NORTH_EAST : SOUTH_WEST;
    constexpr Direction UpLeft    = Us == WHITE ? NORTH_WEST : SOUTH_EAST;
    constexpr Bitboard  Start     = Us == WHITE ? RANK_2     : RANK_7;
    constexpr Bitboard  Promote   = Us == WHITE ? RANK_7     : RANK_2;
    constexpr Bitboard  NoPromote = ~Promote;

    Bitboard not_pinned = ~pinned;

    if (Bitboard promotable = bb(FriendlyPawn) & Promote)
    {
        for (Bitboard b = shift<UpRight>(promotable & (not_pinned | anti_diag(ksq))) & checkmask; b; pop_lsb(b))
        {
            Square to = lsb(b);
            *last++ = make_move<PROMOTION>(to - UpRight, to);
        }
        for (Bitboard b = shift<UpLeft >(promotable & (not_pinned | main_diag(ksq))) & checkmask; b; pop_lsb(b))
        {
            Square to = lsb(b);
            *last++ = make_move<PROMOTION>(to - UpLeft, to);
        }
    }

    Bitboard pawns = bb(FriendlyPawn) & NoPromote;

    last = make_pawn_moves<UpRight>(last, shift<UpRight>(pawns & (not_pinned | anti_diag(ksq))) & checkmask);
    last = make_pawn_moves<UpLeft >(last, shift<UpLeft >(pawns & (not_pinned | main_diag(ksq))) & checkmask);

    Bitboard minor_targets = (bb(Them) ^ bb(EnemyPawn) | enemy_unprotected) & checkmask;
    Bitboard rook_targets  = (bb(EnemyRook) | bb(EnemyQueen) | enemy_unprotected) & checkmask;
    Bitboard queen_targets = (bb(EnemyQueen) | enemy_unprotected) & checkmask;

    for (Bitboard b = bb(FriendlyKnight) & not_pinned; b; pop_lsb(b))
    {
        Square from = lsb(b);
        last = make_moves(last, from, knight_attacks(from) & minor_targets);
    }
    for (Bitboard b = bb(FriendlyBishop) & not_pinned; b; pop_lsb(b))
    {
        Square from = lsb(b);
        last = make_moves(last, from, bishop_attacks(from, occupied) & minor_targets);
    }
    for (Bitboard b = bb(FriendlyBishop) & pinned; b; pop_lsb(b))
    {
        Square from = lsb(b);
        last = make_moves(last, from, bishop_attacks(from, occupied) & minor_targets & pin_mask(ksq, from));
    }
    for (Bitboard b = bb(FriendlyRook) & not_pinned; b; pop_lsb(b))
    {
        Square from = lsb(b);
        last = make_moves(last, from, rook_attacks(from, occupied) & rook_targets);
    }
    for (Bitboard b = bb(FriendlyRook) & pinned; b; pop_lsb(b))
    {
        Square from = lsb(b);
        last = make_moves(last, from, rook_attacks(from, occupied) & rook_targets & pin_mask(ksq, from));
    }
    for (Bitboard b = bb(FriendlyQueen) & not_pinned; b; pop_lsb(b))
    {
        Square from = lsb(b);
        last = make_moves(last, from, queen_attacks(from, occupied) & queen_targets);
    }
    for (Bitboard b = bb(FriendlyQueen) & pinned; b; pop_lsb(b))
    {
        Square from = lsb(b);
        last = make_moves(last, from, queen_attacks(from, occupied) & queen_targets & pin_mask(ksq, from));
    }

    last = make_moves(last, ksq, king_attacks(ksq) & enemy_unprotected);
}

#endif
