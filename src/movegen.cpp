
#include "bitboard.h"
#include "position.h"
#include "types.h"

template<MoveType Type, Direction D>
Move *make_pawn_moves(Move *list, Bitboard attacks)
{
    if constexpr (Type == NORMAL)
    {
        for (;attacks; clear_lsb(attacks))
        {
            Square to = lsb(attacks);
            *list++ = Move(to - D, to);
        }
    }
    else if constexpr (Type == PROMOTION)
    {
        for (;attacks; clear_lsb(attacks))
        {
            Square to = lsb(attacks);
            *list++ = Move::make<KNIGHT_PROMOTION>(to - D, to);
            *list++ = Move::make<BISHOP_PROMOTION>(to - D, to);
            *list++ = Move::make<ROOK_PROMOTION  >(to - D, to);
            *list++ = Move::make<QUEEN_PROMOTION >(to - D, to);
        }
    }

    return list;
}

Move *make_moves(Move *list, Square from, Bitboard to)
{
    for (;to; clear_lsb(to))
        *list++ = Move(from, lsb(to));

    return list;
}

template<Color Us>
Move *generate_moves(Move *list, const Position& pos)
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

    Bitboard enemy_rook_queen   = pos.bb(EnemyQueen) | pos.bb(EnemyRook);
    Bitboard enemy_bishop_queen = pos.bb(EnemyQueen) | pos.bb(EnemyBishop);
    Square   ksq                = lsb(pos.bb(FriendlyKing));
    Bitboard occupied           = pos.occupied();

    Bitboard seen_by_enemy      = pawn_attacks<Them>(pos.bb(EnemyPawn)) | king_attacks(lsb(pos.bb(EnemyKing)));

    for (Bitboard b = pos.bb(EnemyKnight); b; clear_lsb(b)) seen_by_enemy |= knight_attacks(lsb(b));
    for (Bitboard b = enemy_bishop_queen;  b; clear_lsb(b)) seen_by_enemy |= attacks_bb(BISHOP, lsb(b), occupied ^ pos.bb(FriendlyKing));
    for (Bitboard b = enemy_rook_queen;    b; clear_lsb(b)) seen_by_enemy |= attacks_bb(ROOK,   lsb(b), occupied ^ pos.bb(FriendlyKing));

    Bitboard checkmask = knight_attacks(ksq) & pos.bb(EnemyKnight) | pawn_attacks<Us>(ksq) & pos.bb(EnemyPawn);

    for (Bitboard checkers = attacks_bb(BISHOP, ksq, occupied) & enemy_bishop_queen | attacks_bb(ROOK, ksq, occupied) & enemy_rook_queen; checkers; clear_lsb(checkers))
        checkmask |= check_ray(ksq, lsb(checkers));

    if (more_than_one(checkmask & double_check(ksq)))
        return make_moves(list, ksq, king_attacks(ksq) & ~(seen_by_enemy | pos.bb(Us)));

    checkmask = (checkmask | -!checkmask) & ~pos.bb(Us);

    Bitboard pinned = 0;

    for (Bitboard pinners = xray_bb(BISHOP, ksq, occupied) & enemy_bishop_queen | xray_bb(ROOK, ksq, occupied) & enemy_rook_queen; pinners; clear_lsb(pinners))
        pinned |= check_ray(ksq, lsb(pinners));

    constexpr Direction Up      = Us == WHITE ? NORTH      : SOUTH;
    constexpr Direction Up2     = Us == WHITE ? NORTH * 2  : SOUTH * 2;
    constexpr Direction UpRight = Us == WHITE ? NORTH_EAST : SOUTH_WEST;
    constexpr Direction UpLeft  = Us == WHITE ? NORTH_WEST : SOUTH_EAST;
    constexpr Bitboard  Rank3   = Us == WHITE ? RANK_3     : RANK_6;
    constexpr Bitboard  Rank6   = Us == WHITE ? RANK_6     : RANK_3;
    constexpr Bitboard  Rank7   = Us == WHITE ? RANK_7     : RANK_2;

    Bitboard pawns = pos.bb(FriendlyPawn) & ~Rank7;
    Bitboard empty = ~occupied;
    Bitboard e     = shift<Up>(Rank3 & empty) & empty;

    list = make_pawn_moves<NORMAL, UpRight>(list, shift<UpRight>(pawns & (~pinned | anti_diag(ksq))) & pos.bb(Them) & checkmask);
    list = make_pawn_moves<NORMAL, UpLeft >(list, shift<UpLeft >(pawns & (~pinned | main_diag(ksq))) & pos.bb(Them) & checkmask);
    list = make_pawn_moves<NORMAL, Up     >(list, shift<Up     >(pawns & (~pinned | file_bb  (ksq))) & empty    & checkmask);
    list = make_pawn_moves<NORMAL, Up2    >(list, shift<Up2    >(pawns & (~pinned | file_bb  (ksq))) & e        & checkmask);

    if (Bitboard promotable = pos.bb(FriendlyPawn) & Rank7)
    {
        list = make_pawn_moves<PROMOTION, UpRight>(list, shift<UpRight>(promotable & (~pinned | anti_diag(ksq))) & pos.bb(Them) & checkmask);
        list = make_pawn_moves<PROMOTION, UpLeft >(list, shift<UpLeft >(promotable & (~pinned | main_diag(ksq))) & pos.bb(Them) & checkmask);
        list = make_pawn_moves<PROMOTION, Up     >(list, shift<Up     >(promotable &  ~pinned                  ) & empty        & checkmask);
    }
 
    if (shift<UpRight>(pos.bb(FriendlyPawn)) & pos.ep_bb() & Rank6)
    {
        *list = Move::make<ENPASSANT>(pos.ep_sq() - UpRight, pos.ep_sq());
        Bitboard after_ep = occupied ^ square_bb(pos.ep_sq() - UpRight, pos.ep_sq() - Up, pos.ep_sq());
        list += !(attacks_bb(BISHOP, ksq, after_ep) & enemy_bishop_queen | attacks_bb(ROOK, ksq, after_ep) & enemy_rook_queen);
    }
    if (shift<UpLeft>(pos.bb(FriendlyPawn)) & pos.ep_bb() & Rank6)
    {
        *list = Move::make<ENPASSANT>(pos.ep_sq() - UpLeft, pos.ep_sq());
        Bitboard after_ep = occupied ^ square_bb(pos.ep_sq() - UpLeft, pos.ep_sq() - Up, pos.ep_sq());
        list += !(attacks_bb(BISHOP, ksq, after_ep) & enemy_bishop_queen | attacks_bb(ROOK, ksq, after_ep) & enemy_rook_queen);
    }   

    for (Bitboard b = pos.bb(FriendlyKnight) & ~pinned; b; clear_lsb(b))
    {
        Square from = lsb(b);
        list = make_moves(list, from, knight_attacks(from) & checkmask);
    }
    for (Bitboard b = pos.bb(FriendlyBishop) | pos.bb(FriendlyQueen); b; clear_lsb(b))
    {
        Square from = lsb(b);
        list = make_moves(list, from, square_bb(from) & pinned ? attacks_bb(BISHOP, from, occupied) & checkmask & align_mask(ksq, from)
                                                               : attacks_bb(BISHOP, from, occupied) & checkmask);
    }
    for (Bitboard b = pos.bb(FriendlyRook) | pos.bb(FriendlyQueen); b; clear_lsb(b))
    {
        Square from = lsb(b);
        list = make_moves(list, from, square_bb(from) & pinned ? attacks_bb(ROOK, from, occupied) & checkmask & align_mask(ksq, from)
                                                               : attacks_bb(ROOK, from, occupied) & checkmask);
    }

    list = make_moves(list, ksq, king_attacks(ksq) & ~(seen_by_enemy | pos.bb(Us)));

    constexpr Bitboard k_no_atk = Us == WHITE ? square_bb(E1, F1, G1) : square_bb(E8, F8, G8);
    constexpr Bitboard k_no_occ = Us == WHITE ? square_bb(F1, G1)     : square_bb(F8, G8);
    constexpr Bitboard q_no_atk = Us == WHITE ? square_bb(C1, D1, E1) : square_bb(C8, D8, E8);
    constexpr Bitboard q_no_occ = Us == WHITE ? square_bb(B1, C1, D1) : square_bb(B8, C8, D8);

    if (pos.kingside_rights(Us) && !(k_no_atk & seen_by_enemy) && !(k_no_occ & occupied))
        *list++ = Us == WHITE ? Move::make<CASTLING>(E1, G1) : Move::make<CASTLING>(E8, G8);

    if (pos.queenside_rights(Us) && !(q_no_atk & seen_by_enemy) && !(q_no_occ & occupied))
        *list++ = Us == WHITE ? Move::make<CASTLING>(E1, C1) : Move::make<CASTLING>(E8, C8);

    return list;
}

Move *Position::get_moves(Move *list) const
{
    return white_to_move() ? generate_moves<WHITE>(list, *this)
                           : generate_moves<BLACK>(list, *this);
}