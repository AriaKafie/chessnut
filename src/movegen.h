
#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "movelist.h"
#include "bitboard.h"
#include "gamestate.h"
#include "board.h"

#define bb(x) Board::bitboards[x]

ForceInline inline Move* make_moves(Move* movelist, Square from, Bitboard to) {
  for (;to; pop_lsb(to))
    *movelist++ = make_move(from, lsb(to));
  return movelist;
}

template<Direction D>
ForceInline Move* make_pawn_moves(Move* movelist, Bitboard attacks) {
  for (;attacks; pop_lsb(attacks)) {
    Square to = lsb(attacks);
    *movelist++ = make_move(to - D, to);
  }
  return movelist;
}

template<Color Side>
CaptureList<Side>::CaptureList() :
  last(moves)
{
  constexpr Color Enemy          = !Side;
  constexpr Piece EnemyPawn      = piece<Enemy, PAWN>();
  constexpr Piece EnemyKnight    = piece<Enemy, KNIGHT>();
  constexpr Piece EnemyBishop    = piece<Enemy, BISHOP>();
  constexpr Piece EnemyRook      = piece<Enemy, ROOK>();
  constexpr Piece EnemyQueen     = piece<Enemy, QUEEN>();
  constexpr Piece EnemyKing      = piece<Enemy, KING>();
  constexpr Piece FriendlyPawn   = piece<Side,  PAWN>();
  constexpr Piece FriendlyKnight = piece<Side,  KNIGHT>();
  constexpr Piece FriendlyBishop = piece<Side,  BISHOP>();
  constexpr Piece FriendlyRook   = piece<Side,  ROOK>();
  constexpr Piece FriendlyQueen  = piece<Side,  QUEEN>();
  constexpr Piece FriendlyKing   = piece<Side,  KING>();

  // init
  Bitboard friends            = Side == WHITE ? Board::white_pieces : Board::black_pieces;
  Bitboard enemies            = Side == WHITE ? Board::black_pieces : Board::white_pieces;
  Bitboard enemy_rook_queen   = bb(EnemyQueen) | bb(EnemyRook);
  Bitboard enemy_bishop_queen = bb(EnemyQueen) | bb(EnemyBishop);
  Square   ksq                = lsb(bb(FriendlyKing));
  Bitboard occupied           = Board::occupied() ^ square_bb(ksq);

  // seen_by_enemy
  seen_by_enemy = PawnAttacks<Enemy>(bb(EnemyPawn));
  seen_by_enemy |= KingAttacks(lsb(bb(EnemyKing)));
  for (Bitboard b = bb(EnemyKnight); b; pop_lsb(b))
    seen_by_enemy |= KnightAttacks(lsb(b));
  for (Bitboard b = enemy_bishop_queen; b; pop_lsb(b))
    seen_by_enemy |= BishopAttacks(lsb(b), occupied);
  for (Bitboard b = enemy_rook_queen; b; pop_lsb(b))
    seen_by_enemy |= RookAttacks(lsb(b), occupied);
  toggle_square(occupied, ksq);

  Bitboard enemy_unprotected = enemies & ~seen_by_enemy;

  // checkmask
  checkmask = KnightAttacks(ksq) & bb(EnemyKnight);
  checkmask |= PawnAttacks<Side>(ksq) & bb(EnemyPawn);
  for (Bitboard checkers = (BishopAttacks(ksq, occupied) & enemy_bishop_queen) | (RookAttacks(ksq, occupied) & enemy_rook_queen);
  checkers; pop_lsb(checkers))
    checkmask |= CheckRay(ksq, lsb(checkers));
  if (more_than_one(checkmask & DoubleCheck(ksq))) {
    last = make_moves(last, ksq, KingAttacks(ksq) & enemy_unprotected);
    return;
  }
  if (checkmask == 0) checkmask = ALL_SQUARES;

  // pinmask
  Bitboard pinned = 0;
  for (Bitboard pinners = (BishopXray(ksq, occupied) & enemy_bishop_queen) | (RookXray(ksq, occupied) & enemy_rook_queen);
  pinners; pop_lsb(pinners))
    pinned |= CheckRay(ksq, lsb(pinners));

  // pawns
  constexpr Direction UpRight   = Side == WHITE ? NORTH_EAST : SOUTH_WEST;
  constexpr Direction UpLeft    = Side == WHITE ? NORTH_WEST : SOUTH_EAST;
  constexpr Bitboard  Start     = Side == WHITE ? RANK_2     : RANK_7;
  constexpr Bitboard  Promote   = Side == WHITE ? RANK_7     : RANK_2;
  constexpr Bitboard  NoPromote = ~Promote;

  Bitboard not_pinned = ~pinned;

  // promotions
  if (Bitboard promotable = bb(FriendlyPawn) & Promote)
  {
    for (Bitboard b = shift<UpRight>(promotable & (not_pinned | FDiag(ksq))) & enemies & checkmask; b; pop_lsb(b)) {
      Square to = lsb(b);
      *last++ = make_move<PROMOTION>(to - UpRight, to);
    }
    for (Bitboard b = shift<UpLeft >(promotable & (not_pinned | BDiag(ksq))) & enemies & checkmask; b; pop_lsb(b)) {
      Square to = lsb(b);
      *last++ = make_move<PROMOTION>(to - UpLeft, to);
    }
  }

  Bitboard empty      = ~occupied;
  Bitboard pawns      = bb(FriendlyPawn) & NoPromote;

  last = make_pawn_moves<UpRight>(last, shift<UpRight>(pawns & (not_pinned | FDiag(ksq))) & enemies & checkmask);
  last = make_pawn_moves<UpLeft >(last, shift<UpLeft >(pawns & (not_pinned | BDiag(ksq))) & enemies & checkmask);

  // knight, bishop, rook, queen, king
  Bitboard minor_targets = (enemies ^ bb(EnemyPawn) | enemy_unprotected) & checkmask;
  Bitboard rook_targets  = (bb(EnemyRook) | bb(EnemyQueen) | enemy_unprotected) & checkmask;
  Bitboard queen_targets = (bb(EnemyQueen) | enemy_unprotected) & checkmask;

  for (Bitboard b = bb(FriendlyKnight) & not_pinned; b; pop_lsb(b)) {
    Square from = lsb(b);
    last = make_moves(last, from, KnightAttacks(from) & minor_targets);
  }
  for (Bitboard b = bb(FriendlyBishop) & not_pinned; b; pop_lsb(b)) {
    Square from = lsb(b);
    last = make_moves(last, from, BishopAttacks(from, occupied) & minor_targets);
  }
  for (Bitboard b = bb(FriendlyBishop) & pinned; b; pop_lsb(b)) {
    Square from = lsb(b);
    last = make_moves(last, from, BishopAttacks(from, occupied) & minor_targets & PinMask(ksq, from));
  }
  for (Bitboard b = bb(FriendlyRook) & not_pinned; b; pop_lsb(b)) {
    Square from = lsb(b);
    last = make_moves(last, from, RookAttacks(from, occupied) & rook_targets);
  }
  for (Bitboard b = bb(FriendlyRook) & pinned; b; pop_lsb(b)) {
    Square from = lsb(b);
    last = make_moves(last, from, RookAttacks(from, occupied) & rook_targets & PinMask(ksq, from));
  }
  for (Bitboard b = bb(FriendlyQueen) & not_pinned; b; pop_lsb(b)) {
    Square from = lsb(b);
    last = make_moves(last, from, QueenAttacks(from, occupied) & queen_targets);
  }
  for (Bitboard b = bb(FriendlyQueen) & pinned; b; pop_lsb(b)) {
    Square from = lsb(b);
    last = make_moves(last, from, QueenAttacks(from, occupied) & queen_targets & PinMask(ksq, from));
  }

  last = make_moves(last, ksq, KingAttacks(ksq) & enemy_unprotected);

}

template<Color Side>
MoveList<Side>::MoveList(bool ep_enabled) :
  last(moves)
{
  constexpr Color Enemy          = !Side;
  constexpr Piece EnemyPawn      = piece<Enemy, PAWN>();
  constexpr Piece EnemyKnight    = piece<Enemy, KNIGHT>();
  constexpr Piece EnemyBishop    = piece<Enemy, BISHOP>();
  constexpr Piece EnemyRook      = piece<Enemy, ROOK>();
  constexpr Piece EnemyQueen     = piece<Enemy, QUEEN>();
  constexpr Piece EnemyKing      = piece<Enemy, KING>();
  constexpr Piece FriendlyPawn   = piece<Side,  PAWN>();
  constexpr Piece FriendlyKnight = piece<Side,  KNIGHT>();
  constexpr Piece FriendlyBishop = piece<Side,  BISHOP>();
  constexpr Piece FriendlyRook   = piece<Side,  ROOK>();
  constexpr Piece FriendlyQueen  = piece<Side,  QUEEN>();
  constexpr Piece FriendlyKing   = piece<Side,  KING>();

  // init
  Bitboard enemy_rook_queen   = bb(EnemyQueen) | bb(EnemyRook);
  Bitboard enemy_bishop_queen = bb(EnemyQueen) | bb(EnemyBishop);
  Bitboard friends            = Side == WHITE ? Board::white_pieces : Board::black_pieces;
  Square   ksq                = lsb(bb(FriendlyKing));
  Bitboard occupied           = Board::occupied() ^ square_bb(ksq);

  // seen_by_enemy
  seen_by_enemy = PawnAttacks<Enemy>(bb(EnemyPawn));
  seen_by_enemy |= KingAttacks(lsb(bb(EnemyKing)));
  for (Bitboard b = bb(EnemyKnight); b; pop_lsb(b))
    seen_by_enemy |= KnightAttacks(lsb(b));
  for (Bitboard b = enemy_bishop_queen; b; pop_lsb(b))
    seen_by_enemy |= BishopAttacks(lsb(b), occupied);
  for (Bitboard b = enemy_rook_queen; b; pop_lsb(b))
    seen_by_enemy |= RookAttacks(lsb(b), occupied);
  toggle_square(occupied, ksq);

  // checkmask
  checkmask = KnightAttacks(ksq) & bb(EnemyKnight);
  checkmask |= PawnAttacks<Side>(ksq) & bb(EnemyPawn);
  for (Bitboard checkers = (BishopAttacks(ksq, occupied) & enemy_bishop_queen) | (RookAttacks(ksq, occupied) & enemy_rook_queen);
  checkers; pop_lsb(checkers))
    checkmask |= CheckRay(ksq, lsb(checkers));
  if (more_than_one(checkmask & DoubleCheck(ksq))) {
    last = make_moves(last, ksq, KingAttacks(ksq) & ~(seen_by_enemy | friends));
    return;
  }
  if (checkmask == 0) checkmask = ALL_SQUARES;

  // pinmask
  Bitboard pinned = 0;
  for (Bitboard pinners = (BishopXray(ksq, occupied) & enemy_bishop_queen) | (RookXray(ksq, occupied) & enemy_rook_queen);
  pinners; pop_lsb(pinners))
    pinned |= CheckRay(ksq, lsb(pinners));

  // pawns
  constexpr Direction Up        = Side == WHITE ? NORTH      : SOUTH;
  constexpr Direction Up2       = Side == WHITE ? NORTHNORTH : SOUTHSOUTH;
  constexpr Direction UpRight   = Side == WHITE ? NORTH_EAST : SOUTH_WEST;
  constexpr Direction UpLeft    = Side == WHITE ? NORTH_WEST : SOUTH_EAST;
  constexpr Bitboard  FriendEP  = Side == WHITE ? RANK_3     : RANK_6;
  constexpr Bitboard  EnemyEP   = Side == WHITE ? RANK_6     : RANK_3;
  constexpr Bitboard  Start     = Side == WHITE ? RANK_2     : RANK_7;
  constexpr Bitboard  Promote   = Side == WHITE ? RANK_7     : RANK_2;
  constexpr Bitboard  NoPromote = ~Promote;

  Bitboard enemies    = Side == WHITE ? Board::black_pieces : Board::white_pieces;
  Bitboard empty      = ~occupied;
  Bitboard not_pinned = ~pinned;
  Bitboard pawns      = bb(FriendlyPawn) & NoPromote;
  Bitboard e          = shift<Up>(FriendEP & empty) & empty;

  last = make_pawn_moves<UpRight>(last, shift<UpRight>(pawns & (not_pinned | FDiag(ksq))) & enemies & checkmask);
  last = make_pawn_moves<UpLeft >(last, shift<UpLeft >(pawns & (not_pinned | BDiag(ksq))) & enemies & checkmask);
  last = make_pawn_moves<Up     >(last, shift<Up     >(pawns & (not_pinned | File (ksq))) & empty   & checkmask);
  last = make_pawn_moves<Up2    >(last, shift<Up2    >(pawns & (not_pinned | File (ksq))) & e       & checkmask);

  // promotions
  if (Bitboard promotable = bb(FriendlyPawn) & Promote)
  {
    for (Bitboard b = shift<UpRight>(promotable & (not_pinned | FDiag(ksq))) & enemies & checkmask; b; pop_lsb(b)) {
      Square to = lsb(b);
      *last++ = make_move<PROMOTION>(to - UpRight, to);
    }
    for (Bitboard b = shift<UpLeft >(promotable & (not_pinned | BDiag(ksq))) & enemies & checkmask; b; pop_lsb(b)) {
      Square to = lsb(b);
      *last++ = make_move<PROMOTION>(to - UpLeft, to);
    }
    for (Bitboard b = shift<Up>(promotable & not_pinned) & empty & checkmask; b; pop_lsb(b)) {
      Square to = lsb(b);
      *last++ = make_move<PROMOTION>(to - Up, to);
    }
  }

  // en passant
  if (ep_enabled) {
    Bitboard ep_square = EnemyEP & GameState::current_ep_square();
    if (Bitboard b = shift<UpRight>(bb(FriendlyPawn)) & ep_square) {
      Square to = lsb(b);
      *last++ = make_move<ENPASSANT>(to - UpRight, to);
      Bitboard ep_toggle = b | shift<-UpRight>(b) | shift<-Up>(b);
      Bitboard o = occupied ^ ep_toggle;
      Bitboard slider_checks = 
        (BishopAttacks(ksq, o) & enemy_bishop_queen)
        | (RookAttacks(ksq, o) & enemy_rook_queen);
      if (slider_checks) last--;
    }
    if (Bitboard b = shift<UpLeft>(bb(FriendlyPawn)) & ep_square)  {
      Square to = lsb(b);
      *last++ = make_move<ENPASSANT>(to - UpLeft, to);
      Bitboard ep_toggle = b | shift<-UpLeft>(b) | shift<-Up>(b);
      Bitboard o = occupied ^ ep_toggle;
      Bitboard slider_checks = 
        (BishopAttacks(ksq, o) & enemy_bishop_queen)
        | (RookAttacks(ksq, o) & enemy_rook_queen);
      if (slider_checks) last--;
    }
  }

  // knight, bishop, rook, queen, king
  Bitboard friendly_rook_queen = bb(FriendlyQueen) | bb(FriendlyRook);
  Bitboard friendly_bishop_queen = bb(FriendlyQueen) | bb(FriendlyBishop);
  Bitboard legal = checkmask & ~friends;

  for (Bitboard b = bb(FriendlyKnight) & not_pinned; b; pop_lsb(b)) {
    Square from = lsb(b);
    last = make_moves(last, from, KnightAttacks(from) & legal);
  }
  for (Bitboard b = friendly_bishop_queen & not_pinned; b; pop_lsb(b)) {
    Square from = lsb(b);
    last = make_moves(last, from, BishopAttacks(from, occupied) & legal);
  }
  for (Bitboard b = friendly_bishop_queen & pinned; b; pop_lsb(b)) {
    Square from = lsb(b);
    last = make_moves(last, from, BishopAttacks(from, occupied) & legal & PinMask(ksq, from));
  }
  for (Bitboard b = friendly_rook_queen & not_pinned; b; pop_lsb(b)) {
    Square from = lsb(b);
    last = make_moves(last, from, RookAttacks(from, occupied) & legal);
  }
  for (Bitboard b = friendly_rook_queen & pinned; b; pop_lsb(b)) {
    Square from = lsb(b);
    last = make_moves(last, from, RookAttacks(from, occupied) & legal & PinMask(ksq, from));
  }

  last = make_moves(last, ksq, KingAttacks(ksq) & ~(seen_by_enemy | friends));

  if (~checkmask) return;

  //castling
  constexpr Bitboard QueenOcc  = Side == WHITE ? square_bb(B1, C1, D1) : square_bb(B8, C8, D8);
  constexpr Bitboard KingBan   = Side == WHITE ? square_bb(F1, G1)     : square_bb(F8, G8);
  constexpr Bitboard QueenAtk  = Side == WHITE ? square_bb(C1, D1)     : square_bb(C8, D8);
  constexpr Bitboard KingHash  = Side == WHITE ? 8                     : 2;
  constexpr Bitboard QueenHash = Side == WHITE ? 4                     : 1;
            Bitboard rights_k  = Side == WHITE ? GameState::rights_K() : GameState::rights_k();
            Bitboard rights_q  = Side == WHITE ? GameState::rights_Q() : GameState::rights_q();
  constexpr Move     SCASTLE   = Side == WHITE ? W_SCASTLE             : B_SCASTLE;
  constexpr Move     LCASTLE   = Side == WHITE ? W_LCASTLE             : B_LCASTLE;
  
  *last = SCASTLE;
  uint64_t key = ((occupied | seen_by_enemy) & KingBan) | rights_k;
  last += key == KingHash;
  *last = LCASTLE;
  key = (occupied & QueenOcc) | (seen_by_enemy & QueenAtk) | rights_q;
  last += key == QueenHash;
  
}

#undef bb

#endif
