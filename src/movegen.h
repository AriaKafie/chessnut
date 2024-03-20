
#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "movelist.h"
#include "bitboard.h"
#include "gamestate.h"
#include "board.h"

ForceInline inline Move* make_moves(Move* list, Square from, Bitboard to) {
  for (;to; pop_lsb(to))
    *list++ = make_move(from, lsb(to));
  return list;
}

template<Direction D>
ForceInline Move* make_pawn_moves(Move* list, Bitboard attacks) {
  for (;attacks; pop_lsb(attacks)) {
    Square to = lsb(attacks);
    *list++ = make_move(to - D, to);
  }
  return list;
}

template<Color Us>
CaptureList<Us>::CaptureList() :
  last(moves)
{
  constexpr Color Them           = !Us;
  constexpr Piece EnemyPawn      = make_piece(Them, PAWN);
  constexpr Piece EnemyKnight    = make_piece(Them, KNIGHT);
  constexpr Piece EnemyBishop    = make_piece(Them, BISHOP);
  constexpr Piece EnemyRook      = make_piece(Them, ROOK);
  constexpr Piece EnemyQueen     = make_piece(Them, QUEEN);
  constexpr Piece EnemyKing      = make_piece(Them, KING);
  constexpr Piece FriendlyPawn   = make_piece(Us,   PAWN);
  constexpr Piece FriendlyKnight = make_piece(Us,   KNIGHT);
  constexpr Piece FriendlyBishop = make_piece(Us,   BISHOP);
  constexpr Piece FriendlyRook   = make_piece(Us,   ROOK);
  constexpr Piece FriendlyQueen  = make_piece(Us,   QUEEN);
  constexpr Piece FriendlyKing   = make_piece(Us,   KING);

  Bitboard enemy_rook_queen   = bb(EnemyQueen) | bb(EnemyRook);
  Bitboard enemy_bishop_queen = bb(EnemyQueen) | bb(EnemyBishop);
  Square   ksq                = lsb(bb(FriendlyKing));
  Bitboard occupied           = occupied_bb() ^ square_bb(ksq);

  seen_by_enemy = pawn_attacks<Them>(bb(EnemyPawn)) | king_attacks(lsb(bb(EnemyKing)));
  for (Bitboard b = bb(EnemyKnight); b; pop_lsb(b))
    seen_by_enemy |= knight_attacks(lsb(b));
  for (Bitboard b = enemy_bishop_queen; b; pop_lsb(b))
    seen_by_enemy |= bishop_attacks(lsb(b), occupied);
  for (Bitboard b = enemy_rook_queen; b; pop_lsb(b))
    seen_by_enemy |= rook_attacks(lsb(b), occupied);

  toggle_square(occupied, ksq);

  Bitboard enemy_unprotected = bb(Them) &~ seen_by_enemy;

  checkmask = knight_attacks(ksq) & bb(EnemyKnight) | pawn_attacks<Us>(ksq) & bb(EnemyPawn);
  for (Bitboard checkers = bishop_attacks(ksq, occupied) & enemy_bishop_queen | rook_attacks(ksq, occupied) & enemy_rook_queen; checkers; pop_lsb(checkers))
    checkmask |= check_ray(ksq, lsb(checkers));
  if (more_than_one(checkmask & double_check(ksq))) {
    last = make_moves(last, ksq, king_attacks(ksq) & enemy_unprotected);
    return;
  }
  if (checkmask == 0) checkmask = ALL_SQUARES;

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
    for (Bitboard b = shift<UpRight>(promotable & (not_pinned | anti_diag(ksq))) & checkmask; b; pop_lsb(b)) {
      Square to = lsb(b);
      *last++ = make_move<PROMOTION>(to - UpRight, to);
    }
    for (Bitboard b = shift<UpLeft >(promotable & (not_pinned | main_diag(ksq))) & checkmask; b; pop_lsb(b)) {
      Square to = lsb(b);
      *last++ = make_move<PROMOTION>(to - UpLeft, to);
    }
  }

  Bitboard pawns = bb(FriendlyPawn) & NoPromote;

  last = make_pawn_moves<UpRight>(last, shift<UpRight>(pawns & (not_pinned | anti_diag(ksq))) & checkmask);
  last = make_pawn_moves<UpLeft >(last, shift<UpLeft >(pawns & (not_pinned | main_diag(ksq))) & checkmask);

  /*Bitboard minor_targets = (bb(Them) ^ bb(EnemyPawn) | enemy_unprotected) & checkmask;
  Bitboard rook_targets  = (bb(EnemyRook) | bb(EnemyQueen) | enemy_unprotected) & checkmask;
  Bitboard queen_targets = (bb(EnemyQueen) | enemy_unprotected) & checkmask;

  for (Bitboard b = bb(FriendlyKnight) & not_pinned; b; pop_lsb(b)) {
    Square from = lsb(b);
    last = make_moves(last, from, knight_attacks(from) & minor_targets);
  }
  for (Bitboard b = bb(FriendlyBishop) & not_pinned; b; pop_lsb(b)) {
    Square from = lsb(b);
    last = make_moves(last, from, bishop_attacks(from, occupied) & minor_targets);
  }
  for (Bitboard b = bb(FriendlyBishop) & pinned; b; pop_lsb(b)) {
    Square from = lsb(b);
    last = make_moves(last, from, bishop_attacks(from, occupied) & minor_targets & pin_mask(ksq, from));
  }
  for (Bitboard b = bb(FriendlyRook) & not_pinned; b; pop_lsb(b)) {
    Square from = lsb(b);
    last = make_moves(last, from, rook_attacks(from, occupied) & rook_targets);
  }
  for (Bitboard b = bb(FriendlyRook) & pinned; b; pop_lsb(b)) {
    Square from = lsb(b);
    last = make_moves(last, from, rook_attacks(from, occupied) & rook_targets & pin_mask(ksq, from));
  }
  for (Bitboard b = bb(FriendlyQueen) & not_pinned; b; pop_lsb(b)) {
    Square from = lsb(b);
    last = make_moves(last, from, QueenAttacks(from, occupied) & queen_targets);
  }
  for (Bitboard b = bb(FriendlyQueen) & pinned; b; pop_lsb(b)) {
    Square from = lsb(b);
    last = make_moves(last, from, QueenAttacks(from, occupied) & queen_targets & pin_mask(ksq, from));
  }*/

  Bitboard friendly_rook_queen   = bb(FriendlyQueen) | bb(FriendlyRook);
  Bitboard friendly_bishop_queen = bb(FriendlyQueen) | bb(FriendlyBishop);

  for (Bitboard b = bb(FriendlyKnight) & not_pinned; b; pop_lsb(b)) {
    Square from = lsb(b);
    last = make_moves(last, from, knight_attacks(from) & checkmask);
  }
  for (Bitboard b = friendly_bishop_queen & not_pinned; b; pop_lsb(b)) {
    Square from = lsb(b);
    last = make_moves(last, from, bishop_attacks(from, occupied) & checkmask);
  }
  for (Bitboard b = friendly_bishop_queen & pinned; b; pop_lsb(b)) {
    Square from = lsb(b);
    last = make_moves(last, from, bishop_attacks(from, occupied) & checkmask & pin_mask(ksq, from));
  }
  for (Bitboard b = friendly_rook_queen & not_pinned; b; pop_lsb(b)) {
    Square from = lsb(b);
    last = make_moves(last, from, rook_attacks(from, occupied) & checkmask);
  }
  for (Bitboard b = friendly_rook_queen & pinned; b; pop_lsb(b)) {
    Square from = lsb(b);
    last = make_moves(last, from, rook_attacks(from, occupied) & checkmask & pin_mask(ksq, from));
  }

  last = make_moves(last, ksq, king_attacks(ksq) & enemy_unprotected);

}

template<Color Us>
MoveList<Us>::MoveList(bool ep_enabled) :
  last(moves)
{
  constexpr Color Them           = !Us;
  constexpr Piece EnemyPawn      = make_piece(Them, PAWN);
  constexpr Piece EnemyKnight    = make_piece(Them, KNIGHT);
  constexpr Piece EnemyBishop    = make_piece(Them, BISHOP);
  constexpr Piece EnemyRook      = make_piece(Them, ROOK);
  constexpr Piece EnemyQueen     = make_piece(Them, QUEEN);
  constexpr Piece EnemyKing      = make_piece(Them, KING);
  constexpr Piece FriendlyPawn   = make_piece(Us,   PAWN);
  constexpr Piece FriendlyKnight = make_piece(Us,   KNIGHT);
  constexpr Piece FriendlyBishop = make_piece(Us,   BISHOP);
  constexpr Piece FriendlyRook   = make_piece(Us,   ROOK);
  constexpr Piece FriendlyQueen  = make_piece(Us,   QUEEN);
  constexpr Piece FriendlyKing   = make_piece(Us,   KING);

  Bitboard enemy_rook_queen   = bb(EnemyQueen) | bb(EnemyRook);
  Bitboard enemy_bishop_queen = bb(EnemyQueen) | bb(EnemyBishop);
  Square   ksq                = lsb(bb(FriendlyKing));
  Bitboard occupied           = occupied_bb() ^ square_bb(ksq);

  seen_by_enemy = pawn_attacks<Them>(bb(EnemyPawn)) | king_attacks(lsb(bb(EnemyKing)));
  for (Bitboard b = bb(EnemyKnight); b; pop_lsb(b))
    seen_by_enemy |= knight_attacks(lsb(b));
  for (Bitboard b = enemy_bishop_queen; b; pop_lsb(b))
    seen_by_enemy |= bishop_attacks(lsb(b), occupied);
  for (Bitboard b = enemy_rook_queen; b; pop_lsb(b))
    seen_by_enemy |= rook_attacks(lsb(b), occupied);

  toggle_square(occupied, ksq);

  checkmask = knight_attacks(ksq) & bb(EnemyKnight) | pawn_attacks<Us>(ksq) & bb(EnemyPawn);
  for (Bitboard checkers = bishop_attacks(ksq, occupied) & enemy_bishop_queen | rook_attacks(ksq, occupied) & enemy_rook_queen; checkers; pop_lsb(checkers))
    checkmask |= check_ray(ksq, lsb(checkers));
  if (more_than_one(checkmask & double_check(ksq))) {
    last = make_moves(last, ksq, king_attacks(ksq) & ~(seen_by_enemy | bb(Us)));
    return;
  }
  if (checkmask == 0) checkmask = ALL_SQUARES;

  Bitboard pinned = 0;
  for (Bitboard pinners = bishop_xray(ksq, occupied) & enemy_bishop_queen | rook_xray(ksq, occupied) & enemy_rook_queen; pinners; pop_lsb(pinners))
    pinned |= check_ray(ksq, lsb(pinners));

  constexpr Direction Up        = Us == WHITE ? NORTH      : SOUTH;
  constexpr Direction Up2       = Us == WHITE ? NORTHNORTH : SOUTHSOUTH;
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
    for (Bitboard b = shift<UpRight>(promotable & (not_pinned | anti_diag(ksq))) & bb(Them) & checkmask; b; pop_lsb(b)) {
      Square to = lsb(b);
      *last++ = make_move<PROMOTION>(to - UpRight, to);
    }
    for (Bitboard b = shift<UpLeft >(promotable & (not_pinned | main_diag(ksq))) & bb(Them) & checkmask; b; pop_lsb(b)) {
      Square to = lsb(b);
      *last++ = make_move<PROMOTION>(to - UpLeft, to);
    }
    for (Bitboard b = shift<Up>(promotable & not_pinned) & empty & checkmask; b; pop_lsb(b)) {
      Square to = lsb(b);
      *last++ = make_move<PROMOTION>(to - Up, to);
    }
  }

  if (ep_enabled) {
    Bitboard ep_square = EnemyEP & GameState::current_ep_square();
    if (Bitboard b = shift<UpRight>(bb(FriendlyPawn)) & ep_square) {
      Square to = lsb(b);
      *last++ = make_move<ENPASSANT>(to - UpRight, to);
      Bitboard ep_toggle = b | shift<-UpRight>(b) | shift<-Up>(b);
      Bitboard o = occupied ^ ep_toggle;
      Bitboard slider_checks = bishop_attacks(ksq, o) & enemy_bishop_queen | rook_attacks(ksq, o) & enemy_rook_queen;
      if (slider_checks)
        last--;
    }
    if (Bitboard b = shift<UpLeft >(bb(FriendlyPawn)) & ep_square) {
      Square to = lsb(b);
      *last++ = make_move<ENPASSANT>(to - UpLeft, to);
      Bitboard ep_toggle = b | shift<-UpLeft>(b) | shift<-Up>(b);
      Bitboard o = occupied ^ ep_toggle;
      Bitboard slider_checks = bishop_attacks(ksq, o) & enemy_bishop_queen | rook_attacks(ksq, o) & enemy_rook_queen;
      if (slider_checks)
        last--;
    }
  }

  Bitboard friendly_rook_queen   = bb(FriendlyQueen) | bb(FriendlyRook);
  Bitboard friendly_bishop_queen = bb(FriendlyQueen) | bb(FriendlyBishop);

  Bitboard legal = checkmask &~ bb(Us);

  for (Bitboard b = bb(FriendlyKnight) & not_pinned; b; pop_lsb(b)) {
    Square from = lsb(b);
    last = make_moves(last, from, knight_attacks(from) & legal);
  }
  for (Bitboard b = friendly_bishop_queen & not_pinned; b; pop_lsb(b)) {
    Square from = lsb(b);
    last = make_moves(last, from, bishop_attacks(from, occupied) & legal);
  }
  for (Bitboard b = friendly_bishop_queen & pinned; b; pop_lsb(b)) {
    Square from = lsb(b);
    last = make_moves(last, from, bishop_attacks(from, occupied) & legal & pin_mask(ksq, from));
  }
  for (Bitboard b = friendly_rook_queen & not_pinned; b; pop_lsb(b)) {
    Square from = lsb(b);
    last = make_moves(last, from, rook_attacks(from, occupied) & legal);
  }
  for (Bitboard b = friendly_rook_queen & pinned; b; pop_lsb(b)) {
    Square from = lsb(b);
    last = make_moves(last, from, rook_attacks(from, occupied) & legal & pin_mask(ksq, from));
  }

  last = make_moves(last, ksq, king_attacks(ksq) & ~(seen_by_enemy | bb(Us)));

  if (~checkmask) return;

  constexpr Bitboard KingBan  = Us == WHITE ? square_bb(F1, G1)     : square_bb(F8, G8);
  constexpr Bitboard QueenOcc = Us == WHITE ? square_bb(B1, C1, D1) : square_bb(B8, C8, D8);
  constexpr Bitboard QueenAtk = Us == WHITE ? square_bb(C1, D1)     : square_bb(C8, D8);
  constexpr Bitboard KingKey  = Us == WHITE ? 0b1000                : 0b0010;
  constexpr Bitboard QueenKey = Us == WHITE ? 0b0100                : 0b0001;
  constexpr Move     SCASTLE  = Us == WHITE ? W_SCASTLE             : B_SCASTLE;
  constexpr Move     LCASTLE  = Us == WHITE ? W_LCASTLE             : B_LCASTLE;
            
  uint64_t hash;

  *last = SCASTLE;
  hash = (occupied | seen_by_enemy) & KingBan | GameState::kingside_rights<Us>();
  last += !(hash ^ KingKey);

  *last = LCASTLE;
  hash = occupied & QueenOcc | seen_by_enemy & QueenAtk | GameState::queenside_rights<Us>();
  last += !(hash ^ QueenKey);
  
}

#endif

