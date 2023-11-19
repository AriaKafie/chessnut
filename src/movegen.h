
#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "types.h"
#include "bitboard.h"
#include "gamestate.h"

#include <string>

namespace Chess {

#define bb Board::bitboard
constexpr int MOVES_MAX = 128;

template<Color Side>
ForceInline int bulk() {
	
}

ForceInline Move* make_moves(Move* move_list, Square from, Bitboard to) {
	for (;to; pop_lsb(to))
		*move_list++ = make_move(from, lsb(to));
	return move_list;
}

template<Direction D>
ForceInline Move* make_pawn_moves(Move* move_list, Bitboard attacks) {
	for (;attacks; pop_lsb(attacks)) {
		Square to = lsb(attacks);
		*move_list++ = make_move(to - D, to);
	}
	return move_list;
}

template<Color Side>
struct MoveList {

	MoveList();
	const Move* begin() const { return moves; }
	const Move* end() const { return last; }
	size_t length() const { return last - moves; }
	bool incheck() { return ~checkmask; }
	Bitboard seenByEnemy;

private:
	Move moves[MOVES_MAX], *last;
	Bitboard checkmask;

};

template<Color Side>
MoveList<Side>::MoveList() :
	checkmask(0),
	seenByEnemy(0),
	last(moves)
{
	constexpr Piece FriendlyPawn   = piece<Side, PAWN>();
	constexpr Piece FriendlyKnight = piece<Side, KNIGHT>();
	constexpr Piece FriendlyBishop = piece<Side, BISHOP>();
	constexpr Piece FriendlyRook   = piece<Side, ROOK>();
	constexpr Piece FriendlyQueen  = piece<Side, QUEEN>();
	constexpr Piece FriendlyKing   = piece<Side, KING>();
	constexpr Color Enemy = flip<Side>();
	constexpr Piece EnemyPawn      = piece<Enemy, PAWN>();
	constexpr Piece EnemyKnight    = piece<Enemy, KNIGHT>();
	constexpr Piece EnemyBishop    = piece<Enemy, BISHOP>();
	constexpr Piece EnemyRook      = piece<Enemy, ROOK>();
	constexpr Piece EnemyQueen     = piece<Enemy, QUEEN>();
	constexpr Piece EnemyKing      = piece<Enemy, KING>();

	// init
	Bitboard enemyRookQueen   = bb[EnemyQueen] | bb[EnemyRook];
	Bitboard enemyBishopQueen = bb[EnemyQueen] | bb[EnemyBishop];
	Bitboard friends = Side == WHITE ? Board::white_pieces : Board::black_pieces;
	Bitboard occupied = Board::occupied();
	Square ksq = lsb(bb[FriendlyKing]);

	// calculate seenByEnemy
	toggle_square(occupied, ksq);
	seenByEnemy = PawnAttacks<Enemy>(bb[EnemyPawn]);
	seenByEnemy |= KingAttacks(lsb(bb[EnemyKing]));
	for (Bitboard b = bb[EnemyKnight]; b; pop_lsb(b))
		seenByEnemy |= KnightAttacks(lsb(b));
	for (Bitboard b = enemyBishopQueen; b; pop_lsb(b))
		seenByEnemy |= BishopAttacks(lsb(b), occupied);
	for (Bitboard b = enemyRookQueen; b; pop_lsb(b))
		seenByEnemy |= RookAttacks(lsb(b), occupied);
	toggle_square(occupied, ksq);

	// calculate checkmask
	checkmask = KnightAttacks(ksq) & bb[EnemyKnight];
	checkmask |= PawnAttacks<Side>(ksq) & bb[EnemyPawn];
	Bitboard checkers =
		(BishopAttacks(ksq, occupied) & enemyBishopQueen)
		| (RookAttacks(ksq, occupied) & enemyRookQueen);
	for (; checkers; pop_lsb(checkers))
		checkmask |= CheckRay(ksq, lsb(checkers));
	if (more_than_one(checkmask & DoubleCheck(ksq))) {
		last = make_moves(last, ksq, KingAttacks(ksq) & ~(seenByEnemy | friends));
		return;
	}
	if (checkmask == 0) checkmask = ALL_SQUARES;

	// calculate pinmask
	Bitboard pinned = 0;
	Bitboard pinners =
		(BishopXray(ksq, occupied) & enemyBishopQueen)
		| (RookXray(ksq, occupied) & enemyRookQueen);
	for (; pinners; pop_lsb(pinners))
		pinned |= CheckRay(ksq, lsb(pinners));

	// pawn moves
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
	Bitboard pawns;

	// UpRight
	pawns = bb[FriendlyPawn] & NoPromote & (not_pinned | f_diagonal[ksq]);
	last = make_pawn_moves<UpRight>(last, shift<UpRight>(pawns) & enemies & checkmask);

	// UpLeft
	pawns = bb[FriendlyPawn] & NoPromote & (not_pinned | b_diagonal[ksq]);
	last = make_pawn_moves<UpLeft>(last, shift<UpLeft>(pawns) & enemies & checkmask);

	// Up
	pawns = bb[FriendlyPawn] & NoPromote & (not_pinned | file[ksq]);
	last = make_pawn_moves<Up>(last, shift<Up>(pawns) & empty & checkmask);

	// Up2
	Bitboard b = shift<Up>(FriendEP & empty) & empty;
	pawns = bb[FriendlyPawn] & (not_pinned | file[ksq]);
	last = make_pawn_moves<Up2>(last, shift<Up2>(pawns) & b & checkmask);

	// promotions
	if (Bitboard promotable = bb[FriendlyPawn] & Promote)
	{
		pawns = promotable & (not_pinned | f_diagonal[ksq]);
		for (Bitboard b = shift<UpRight>(pawns) & enemies & checkmask; b; pop_lsb(b)) {
			Square to = lsb(b);
			*last++ = make_move<PROMOTION>(to - UpRight, to);
		}
		pawns = promotable & (not_pinned | b_diagonal[ksq]);
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
		if (Bitboard b = shift<UpRight>(bb[FriendlyPawn]) & ep_square) {
			Square to = lsb(b);
			*last++ = make_move<ENPASSANT>(to - UpRight, to);
			Bitboard ep_toggle = b | shift<-UpRight>(b) | shift<-Up>(b);
			Bitboard occ = occupied ^ ep_toggle;
			Bitboard slider_checks =
				(BishopAttacks(ksq, occ) & enemyBishopQueen)
				| (RookAttacks(ksq, occ) & enemyRookQueen);
			if (slider_checks) last--;
		}
		if (Bitboard b = shift<UpLeft>(bb[FriendlyPawn]) & ep_square)  {
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

	// knight, bishop, rook, queen
	Bitboard friendlyRookQueen = bb[FriendlyQueen] | bb[FriendlyRook];
	Bitboard friendlyBishopQueen = bb[FriendlyQueen] | bb[FriendlyBishop];
	Bitboard legal = checkmask & ~friends;

	for (Bitboard b = bb[FriendlyKnight] & not_pinned; b; pop_lsb(b)) {
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

	if constexpr (Side == WHITE) {
		*last = SHORTCASTLE;
		uint64_t legality = ((occupied | seenByEnemy) & 0b110ull) | GameState::rights_K();
		last += legality == 8;
		*last = LONGCASTLE;
		legality = (occupied & 0x70ull) | (seenByEnemy & 0x30ull) | GameState::rights_Q();
		last += legality == 4;
	}
	else {
		*last = SHORTCASTLE;
		uint64_t legality = ((occupied | seenByEnemy) & 0x600000000000000ull) | GameState::rights_k();
		last += legality == 2;
		*last = LONGCASTLE;
		legality = (occupied & 0x7000000000000000ull) | (seenByEnemy & 0x3000000000000000ull) | GameState::rights_q();
		last += legality == 1;
	}

}

#undef bb

}

#endif
