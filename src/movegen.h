
#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "types.h"
#include "bitboard.h"
#include "gamestate.h"
#include "board.h"

#include <string>

#define bb(x) Board::bitboards[x]
constexpr int MAX_MOVES = 128;

inline Move* make_moves(Move* move_list, Square from, Bitboard to) {
  for (;to; pop_lsb(to))
    *move_list++ = make_move(from, lsb(to));
  return move_list;
}

template<Direction D>
Move* make_pawn_moves(Move* move_list, Bitboard attacks) {
  for (;attacks; pop_lsb(attacks)) {
    Square to = lsb(attacks);
    *move_list++ = make_move(to - D, to);
  }
  return move_list;
}

template<Color Side>
class MoveList {

public:
  MoveList();
  const Move* begin() const { return moves; }
  const Move* end() const { return last; }
  size_t length() const { return last - moves; }
  bool incheck() const { return ~checkmask; }
  Bitboard seenByEnemy;

private:
  Move moves[MAX_MOVES], *last;
  Bitboard checkmask;

};

template<Color Side>
MoveList<Side>::MoveList() :
  checkmask  (0),
  seenByEnemy(0),
  last       (moves)
{
  constexpr Color Enemy = flip<Side>();
  constexpr Piece FriendlyPawn   = piece<Side,  PAWN>();
  constexpr Piece FriendlyKnight = piece<Side,  KNIGHT>();
  constexpr Piece FriendlyBishop = piece<Side,  BISHOP>();
  constexpr Piece FriendlyRook   = piece<Side,  ROOK>();
  constexpr Piece FriendlyQueen  = piece<Side,  QUEEN>();
  constexpr Piece FriendlyKing   = piece<Side,  KING>();
  constexpr Piece EnemyPawn      = piece<Enemy, PAWN>();
  constexpr Piece EnemyKnight    = piece<Enemy, KNIGHT>();
  constexpr Piece EnemyBishop    = piece<Enemy, BISHOP>();
  constexpr Piece EnemyRook      = piece<Enemy, ROOK>();
  constexpr Piece EnemyQueen     = piece<Enemy, QUEEN>();
  constexpr Piece EnemyKing      = piece<Enemy, KING>();

  // init
  Bitboard enemyRookQueen = bb(EnemyQueen) | bb(EnemyRook);
  Bitboard enemyBishopQueen = bb(EnemyQueen) | bb(EnemyBishop);
  Bitboard friends = Side == WHITE ? Board::white_pieces : Board::black_pieces;
  Bitboard occupied = Board::occupied();
  Square ksq = lsb(bb(FriendlyKing));

  // seenByEnemy
  toggle_square(occupied, ksq);
  seenByEnemy = PawnAttacks<Enemy>(bb(EnemyPawn));
  seenByEnemy |= KingAttacks(lsb(bb(EnemyKing)));
  for (Bitboard b = bb(EnemyKnight); b; pop_lsb(b))
    seenByEnemy |= KnightAttacks(lsb(b));
  for (Bitboard b = enemyBishopQueen; b; pop_lsb(b))
    seenByEnemy |= BishopAttacks(lsb(b), occupied);
  for (Bitboard b = enemyRookQueen; b; pop_lsb(b))
    seenByEnemy |= RookAttacks(lsb(b), occupied);
  toggle_square(occupied, ksq);

  // checkmask
  checkmask = KnightAttacks(ksq) & bb(EnemyKnight);
  checkmask |= PawnAttacks<Side>(ksq) & bb(EnemyPawn);
  Bitboard checkers =
    (BishopAttacks(ksq, occupied) & enemyBishopQueen)
    | (RookAttacks(ksq, occupied) & enemyRookQueen);
  for (;checkers; pop_lsb(checkers))
    checkmask |= CheckRay(ksq, lsb(checkers));
  if (more_than_one(checkmask & DoubleCheck(ksq))) {
    last = make_moves(last, ksq, KingAttacks(ksq) & ~(seenByEnemy | friends));
    return;
  }
  if (checkmask == 0) checkmask = ALL_SQUARES;

  // pinmask
  Bitboard pinned = 0;
  Bitboard pinners =
    (BishopXray(ksq, occupied) & enemyBishopQueen)
    | (RookXray(ksq, occupied) & enemyRookQueen);
  for (;pinners; pop_lsb(pinners))
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

  Bitboard enemies = Side == WHITE ? Board::black_pieces : Board::white_pieces;
  Bitboard empty = ~occupied;
  Bitboard not_pinned = ~pinned;
  Bitboard pawns = bb(FriendlyPawn) & NoPromote;
  Bitboard e = shift<Up>(FriendEP & empty) & empty;
  
  last = make_pawn_moves<UpRight>(last, shift<UpRight>(pawns & (not_pinned | FDiag(ksq))) & enemies & checkmask);
  last = make_pawn_moves<UpLeft >(last, shift<UpLeft >(pawns & (not_pinned | BDiag(ksq))) & enemies & checkmask);
  last = make_pawn_moves<Up     >(last, shift<Up     >(pawns & (not_pinned | File (ksq))) & empty   & checkmask);
  last = make_pawn_moves<Up2    >(last, shift<Up2    >(pawns & (not_pinned | File (ksq))) & e       & checkmask);

  // promotions
  if (Bitboard promotable = bb(FriendlyPawn) & Promote)
    {
      pawns = promotable & (not_pinned | FDiag(ksq));
      for (Bitboard b = shift<UpRight>(pawns) & enemies & checkmask; b; pop_lsb(b)) {
	Square to = lsb(b);
	*last++ = make_move<PROMOTION>(to - UpRight, to);
      }
      pawns = promotable & (not_pinned | BDiag(ksq));
      for (Bitboard b = shift<UpLeft>(pawns) & enemies & checkmask; b; pop_lsb(b)) {
	Square to = lsb(b);
	*last++ = make_move<PROMOTION>(to - UpLeft, to);
      }
      pawns = promotable & not_pinned;
      for (Bitboard b = shift<Up>(pawns) & empty & checkmask; b; pop_lsb(b)) {
	Square to = lsb(b);
	*last++ = make_move<PROMOTION>(to - Up, to);
      }
    }

  // en passant
  /*
    if (Bitboard ep_square = EnemyEP & GameState::current_ep_square()) {
    if (Bitboard b = shift<UpRight>(bb(FriendlyPawn)) & ep_square) {
    Square to = lsb(b);
    *last++ = make_move<ENPASSANT>(to - UpRight, to);
    Bitboard ep_toggle = b | shift<-UpRight>(b) | shift<-Up>(b);
    Bitboard occ = occupied ^ ep_toggle;
    itboard slider_checks =
    (BishopAttacks(ksq, occ) & enemyBishopQueen)
    | (RookAttacks(ksq, occ) & enemyRookQueen);
    if (slider_checks) last--;
    }
    if (Bitboard b = shift<UpLeft>(bb(FriendlyPawn)) & ep_square)  {
    Square to = lsb(b);
    *last++ = make_move<ENPASSANT>(to - UpLeft, to);
    Bitboard ep_toggle = b | shift<-UpLeft>(b) | shift<-Up>(b);
    Bitboard occ = occupied ^ ep_toggle;
    Bitboard slider_checks =
    (BishopAttacks(ksq, occ) & enemyBishopQueen)
    | (RookAttacks(ksq, occ) & enemyRookQueen);
    if (slider_checks) last--;
    }
    }*/

  // knight, bishop, rook, queen, king
  Bitboard friendlyRookQueen = bb(FriendlyQueen) | bb(FriendlyRook);
  Bitboard friendlyBishopQueen = bb(FriendlyQueen) | bb(FriendlyBishop);
  Bitboard legal = checkmask & ~friends;

  for (Bitboard b = bb(FriendlyKnight) & not_pinned; b; pop_lsb(b)) {
    Square from = lsb(b);
    last = make_moves(last, from, KnightAttacks(from) & legal);
  }
  for (Bitboard b = friendlyBishopQueen & not_pinned; b; pop_lsb(b)) {
    Square from = lsb(b);
    last = make_moves(last, from, BishopAttacks(from, occupied) & legal);
  }
  for (Bitboard b = friendlyBishopQueen & pinned; b; pop_lsb(b)) {
    Square from = lsb(b);
    last = make_moves(last, from, BishopAttacks(from, occupied) & legal & PinMask(ksq, from));
  }
  for (Bitboard b = friendlyRookQueen & not_pinned; b; pop_lsb(b)) {
    Square from = lsb(b);
    last = make_moves(last, from, RookAttacks(from, occupied) & legal);
  }
  for (Bitboard b = friendlyRookQueen & pinned; b; pop_lsb(b)) {
    Square from = lsb(b);
    last = make_moves(last, from, RookAttacks(from, occupied) & legal & PinMask(ksq, from));
  }

  last = make_moves(last, ksq, KingAttacks(ksq) & ~(seenByEnemy | friends));

  if (~checkmask) return;

  //castling
  constexpr Bitboard QueenOcc  = Side == WHITE ? square_bb(B1, C1, D1) : square_bb(B8, C8, D8);
  constexpr Bitboard KingBan   = Side == WHITE ? square_bb(F1, G1)     : square_bb(F8, G8);
  constexpr Bitboard QueenAtk  = Side == WHITE ? square_bb(C1, D1)     : square_bb(C8, D8);
  constexpr Bitboard KingHash  = Side == WHITE ? 8                     : 2;
  constexpr Bitboard QueenHash = Side == WHITE ? 4                     : 1;
            Bitboard rights_k  = Side == WHITE ? GameState::rights_K() : GameState::rights_k();
	    Bitboard rights_q  = Side == WHITE ? GameState::rights_Q() : GameState::rights_q();
  
  *last = SHORTCASTLE;
  uint64_t key = ((occupied | seenByEnemy) & KingBan) | rights_k;
  last += key == KingHash;
  *last = LONGCASTLE;
  key = (occupied & QueenOcc) | (seenByEnemy & QueenAtk) | rights_q;
  last += key == QueenHash;

}

#undef bb

#endif
