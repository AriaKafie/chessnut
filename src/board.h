
#ifndef BOARD_H
#define BOARD_H

#include "bitboard.h"
#include "gamestate.h"
#include "types.h"
#include "zobrist.h"

inline Bitboard bitboards[B_KING + 1];
inline Piece board[SQUARE_NB];

template<Piece P>
Bitboard bb() { return bitboards[P]; }

#define bb(P) bb<P>()

inline Piece piece_on(Square sq) { return board[sq]; }

inline PieceType piece_type_on(Square sq) { return type_of(board[sq]); }

template<Color> void do_move(Move m);
template<Color> void undo_move(Move m, Piece captured);

template<Color> ForceInline void do_capture(Move m);
template<Color> ForceInline void undo_capture(Move m, Piece captured);

inline Bitboard occupied_bb() { return bitboards[WHITE] | bitboards[BLACK]; }

inline void do_legal(Move m) {
  if (GameState::white_to_move)
    do_move<WHITE>(m);
  else
    do_move<BLACK>(m);
  GameState::update(m);
}

inline void undo_legal(Move m, Piece captured) {
  if (GameState::white_to_move)
    undo_move<BLACK>(m, captured);
  else
    undo_move<WHITE>(m, captured);
  GameState::restore();
}

template<Color JustMoved, MoveType Type>
ForceInline void update_castling_rights() {

  constexpr Color Them = !JustMoved;

  constexpr uint8_t ClearRights = JustMoved == WHITE ? 0b0011 : 0b1100;

  constexpr Bitboard FriendlyKingStart = square_bb(JustMoved == WHITE ? E1 : E8);
  constexpr Bitboard EnemyKingStart    = square_bb(JustMoved == WHITE ? E8 : E1);
  constexpr Bitboard FriendlyRookStart =
    JustMoved == WHITE ? square_bb(A1, H1) : square_bb(A8, H8);

  constexpr Piece FriendlyKing = make_piece(JustMoved, KING);
  constexpr Piece FriendlyRook = make_piece(JustMoved, ROOK);
  constexpr Piece EnemyRook    = make_piece(Them     , ROOK);
  
  if constexpr (Type == NORMAL)
    GameState::castling_rights &=
      CastlingPext(bitboards[FriendlyKing] & FriendlyKingStart | bitboards[FriendlyRook] & FriendlyRookStart | bitboards[EnemyRook] | EnemyKingStart);

  else if constexpr (Type == PROMOTION)
    GameState::castling_rights &=
      CastlingPext(bitboards[EnemyRook] | FriendlyKingStart | FriendlyRookStart | EnemyKingStart);

  else if constexpr (Type == SHORTCASTLE || Type == LONGCASTLE)
    GameState::castling_rights &= ClearRights;

}

template<Color Us>
ForceInline void do_capture(Move m) {

  constexpr Color Them  = !Us;
  constexpr Piece Pawn  = make_piece(Us, PAWN);
  constexpr Piece Queen = make_piece(Us, QUEEN);

  const Square from = from_sq(m);
  const Square to   = to_sq(m);

  Bitboard to_bb   = square_bb(to);
  Bitboard from_to = square_bb(from, to);

  switch (type_of(m)) {
  case NORMAL:
    bitboards[board[to]] ^= to_bb;
    bitboards[Them] ^= to_bb;
    bitboards[board[from]] ^= from_to;
    bitboards[Us] ^= from_to;
    board[to] = board[from];
    board[from] = NO_PIECE;
    return;
  case PROMOTION:
    bitboards[board[to]] ^= to_bb;
    bitboards[Them] ^= to_bb;
    bitboards[Pawn] ^= square_bb(from);
    bitboards[Queen] ^= to_bb;
    bitboards[Us] ^= from_to;
    board[to] = Queen;
    board[from] = NO_PIECE;
    return;
  }
}

template<Color Us>
ForceInline void undo_capture(Move m, Piece captured) {

  constexpr Color Them  = !Us;
  constexpr Piece Pawn  = make_piece(Us, PAWN);
  constexpr Piece Queen = make_piece(Us, QUEEN);

  const Square from = from_sq(m);
  const Square to   = to_sq(m);

  Bitboard to_bb   = square_bb(to);
  Bitboard from_to = square_bb(from, to);

  switch (type_of(m)) {
  case NORMAL:
    bitboards[board[to]] ^= from_to;
    bitboards[Us] ^= from_to;
    bitboards[captured] ^= to_bb;
    bitboards[Them] ^= to_bb;
    board[from] = board[to];
    board[to] = captured;
    return;
  case PROMOTION:
    bitboards[Queen] ^= to_bb;
    bitboards[Pawn] ^= square_bb(from);
    bitboards[Us] ^= from_to;
    bitboards[captured] ^= to_bb;
    bitboards[Them] ^= to_bb;
    board[from] = Pawn;
    board[to] = captured;
    return;
  }
}

template<Color Us>
void do_move(Move m) {

  Zobrist::push();

  constexpr Color Them = !Us;

  constexpr Piece Pawn  = make_piece(Us, PAWN);
  constexpr Piece Rook  = make_piece(Us, ROOK);
  constexpr Piece Queen = make_piece(Us, QUEEN);
  constexpr Piece King  = make_piece(Us, KING);

  const Square from = from_sq(m);
  const Square to   = to_sq(m);

  Bitboard to_bb   = square_bb(to);
  Bitboard from_to = square_bb(from, to);

  switch (type_of(m)) {
  case NORMAL:
    Zobrist::key ^= Zobrist::hash[board[from]][from];
    Zobrist::key ^= Zobrist::hash[board[from]][to];
    Zobrist::key ^= Zobrist::hash[board[to]][to];
    Zobrist::key ^= Zobrist::BlackToMove;
    bitboards[board[to]] &= ~to_bb;
    bitboards[Them] &= ~to_bb;
    bitboards[board[from]] ^= from_to;
    bitboards[Us] ^= from_to;
    board[to] = board[from];
    board[from] = NO_PIECE;
    update_castling_rights<Us, NORMAL>();
    return;
  case PROMOTION:
    Zobrist::key ^= Zobrist::hash[Pawn][from];
    Zobrist::key ^= Zobrist::hash[Queen][to];
    Zobrist::key ^= Zobrist::hash[board[to]][to];
    Zobrist::key ^= Zobrist::BlackToMove;
    bitboards[board[to]] &= ~to_bb;
    bitboards[Them] &= ~to_bb;
    bitboards[Pawn] ^= square_bb(from);
    bitboards[Queen] ^= to_bb;
    bitboards[Us] ^= from_to;
    board[to] = Queen;
    board[from] = NO_PIECE;
    update_castling_rights<Us, PROMOTION>();
    return;
  case SHORTCASTLE:
  {
    constexpr Square king_from = Us == WHITE ? E1 : E8;
    constexpr Square king_to   = Us == WHITE ? G1 : G8;
    constexpr Square rook_from = Us == WHITE ? H1 : H8;
    constexpr Square rook_to   = Us == WHITE ? F1 : F8;

    constexpr Bitboard king_from_to = square_bb(king_from, king_to);
    constexpr Bitboard rook_from_to = square_bb(rook_from, rook_to);

    Zobrist::key ^= Zobrist::hash[King][king_from];
    Zobrist::key ^= Zobrist::hash[King][king_to];
    Zobrist::key ^= Zobrist::hash[Rook][rook_from];
    Zobrist::key ^= Zobrist::hash[Rook][rook_to];
    Zobrist::key ^= Zobrist::BlackToMove;
    bitboards[King] ^= king_from_to;
    bitboards[Rook] ^= rook_from_to;
    bitboards[Us] ^= king_from_to ^ rook_from_to;
    board[king_from] = NO_PIECE;
    board[rook_from] = NO_PIECE;
    board[king_to] = King;
    board[rook_to] = Rook;
    update_castling_rights<Us, SHORTCASTLE>();
  }
    return;
  case LONGCASTLE:
  {
    constexpr Square king_from = Us == WHITE ? E1 : E8;
    constexpr Square king_to   = Us == WHITE ? C1 : C8;
    constexpr Square rook_from = Us == WHITE ? A1 : A8;
    constexpr Square rook_to   = Us == WHITE ? D1 : D8;

    constexpr Bitboard king_from_to = square_bb(king_from, king_to);
    constexpr Bitboard rook_from_to = square_bb(rook_from, rook_to);

    Zobrist::key ^= Zobrist::hash[King][king_from];
    Zobrist::key ^= Zobrist::hash[King][king_to];
    Zobrist::key ^= Zobrist::hash[Rook][rook_from];
    Zobrist::key ^= Zobrist::hash[Rook][rook_to];
    Zobrist::key ^= Zobrist::BlackToMove;
    bitboards[King] ^= king_from_to;
    bitboards[Rook] ^= rook_from_to;
    bitboards[Us] ^= king_from_to ^ rook_from_to;
    board[king_from] = NO_PIECE;
    board[rook_from] = NO_PIECE;
    board[king_to] = King;
    board[rook_to] = Rook;
    update_castling_rights<Us, LONGCASTLE>();
  }
  return;
  case ENPASSANT:
    constexpr Piece  EPawn = make_piece(Them, PAWN);
              Square capsq = to + (Us == WHITE ? SOUTH : NORTH);
    Zobrist::key ^= Zobrist::hash[Pawn][from];
    Zobrist::key ^= Zobrist::hash[Pawn][to];
    Zobrist::key ^= Zobrist::hash[EPawn][capsq];
    Zobrist::key ^= Zobrist::BlackToMove;
    bitboards[Pawn] ^= from_to;
    bitboards[EPawn] ^= square_bb(capsq);
    bitboards[Us] ^= from_to;
    bitboards[Them] ^= square_bb(capsq);
    board[from] = NO_PIECE;
    board[to] = Pawn;
    board[capsq] = NO_PIECE;
    return;
  }
}

template<Color Us>
void undo_move(Move m, Piece captured) {

  Zobrist::pop();

  constexpr Color Them = !Us;

  constexpr Piece Pawn  = make_piece(Us, PAWN);
  constexpr Piece Rook  = make_piece(Us, ROOK);
  constexpr Piece Queen = make_piece(Us, QUEEN);
  constexpr Piece King  = make_piece(Us, KING);

  const Square from = from_sq(m);
  const Square to   = to_sq(m);

  Bitboard to_bb      = square_bb(to);
  Bitboard from_to    = square_bb(from, to);
  Bitboard capture_bb = to_bb * bool(captured);

  switch (type_of(m)) {
  case NORMAL:
    bitboards[board[to]] ^= from_to;
    bitboards[Us] ^= from_to;
    bitboards[captured] ^= capture_bb;
    bitboards[Them] ^= capture_bb;
    board[from] = board[to];
    board[to] = captured;
    return;
  case PROMOTION:
    bitboards[Queen] ^= to_bb;
    bitboards[Pawn] ^= square_bb(from);
    bitboards[Us] ^= from_to;
    bitboards[captured] ^= capture_bb;
    bitboards[Them] ^= capture_bb;
    board[to] = captured;
    board[from] = Pawn;
    return;
  case SHORTCASTLE:
  {
    constexpr Square king_from = Us == WHITE ? E1 : E8;
    constexpr Square king_to   = Us == WHITE ? G1 : G8;
    constexpr Square rook_from = Us == WHITE ? H1 : H8;
    constexpr Square rook_to   = Us == WHITE ? F1 : F8;

    constexpr Bitboard king_from_to = square_bb(king_from, king_to);
    constexpr Bitboard rook_from_to = square_bb(rook_from, rook_to);

    bitboards[King] ^= king_from_to;
    bitboards[Rook] ^= rook_from_to;
    bitboards[Us] ^= king_from_to ^ rook_from_to;
    board[king_to] = NO_PIECE;
    board[rook_to] = NO_PIECE;
    board[king_from] = King;
    board[rook_from] = Rook;
  }
  return;
  case LONGCASTLE:
  {
    constexpr Square king_from = Us == WHITE ? E1 : E8;
    constexpr Square king_to   = Us == WHITE ? C1 : C8;
    constexpr Square rook_from = Us == WHITE ? A1 : A8;
    constexpr Square rook_to   = Us == WHITE ? D1 : D8;

    constexpr Bitboard king_from_to = square_bb(king_from, king_to);
    constexpr Bitboard rook_from_to = square_bb(rook_from, rook_to);

    bitboards[King] ^= king_from_to;
    bitboards[Rook] ^= rook_from_to;
    bitboards[Us] ^= king_from_to ^ rook_from_to;
    board[king_to] = NO_PIECE;
    board[rook_to] = NO_PIECE;
    board[king_from] = King;
    board[rook_from] = Rook;
  }
  return;
  case ENPASSANT:
    constexpr Piece  EPawn = make_piece(Them, PAWN);
              Square capsq = to + (Us == WHITE ? SOUTH : NORTH);

    bitboards[Pawn] ^= from_to;
    bitboards[Us] ^= from_to;
    bitboards[EPawn] ^= square_bb(capsq);
    bitboards[Them] ^= square_bb(capsq);
    board[to] = NO_PIECE;
    board[from] = Pawn;
    board[capsq] = EPawn;
    return;
  }
}

#endif
