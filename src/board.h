
#ifndef BOARD_H
#define BOARD_H

#include "bitboard.h"
#include "gamestate.h"
#include "types.h"

extern Bitboard bitboards[];
extern Piece board[];

template<PieceType PT>
Bitboard bb() {
  return bitboards[PT];
}

template<Piece P>
Bitboard piece_bb() {
  constexpr Color C = color_of(P);
  constexpr PieceType PT = type_of(P);
  return bitboards[PT] & bitboards[C];
}

#define bb(P) piece_bb<P>()

inline Piece piece_on(Square sq) { return board[sq]; }

inline PieceType piece_type_on(Square sq) { return type_of(board[sq]); }

template<Color> void do_move(Move m);
template<Color> void undo_move(Move m, Piece captured);

template<Color> void do_capture(Move m);
template<Color> void undo_capture(Move m, Piece captured);

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
      CastlingPext(piece_bb<FriendlyKing>() & FriendlyKingStart | piece_bb<FriendlyRook>() & FriendlyRookStart | piece_bb<EnemyRook>() | EnemyKingStart);

  else if constexpr (Type == PROMOTION)
    GameState::castling_rights &=
      CastlingPext(piece_bb<EnemyRook>() | FriendlyKingStart | FriendlyRookStart | EnemyKingStart);

  else if constexpr (Type == SHORTCASTLE || Type == LONGCASTLE)
    GameState::castling_rights &= ClearRights;

}

#endif
