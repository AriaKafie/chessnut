
#ifndef POSITION_H
#define POSITION_H

#include "bitboard.h"
#include "types.h"

#include <string>

extern Bitboard bitboards[];
extern Piece board[];

inline Stack<Piece> piece_stack;

namespace Zobrist {
  constexpr uint64_t Side = 17200288208102703589ull;
  extern uint64_t hash[B_KING + 1][SQUARE_NB];
  extern uint64_t key;
}

namespace Position {

void init();
void set(const std::string& fen);

inline Color      side_to_move;
inline Color      us;
inline Color      them;
inline GamePhase  gamephase;
inline StateImage st;

inline bool white_to_move() { return side_to_move == WHITE; }
inline bool black_to_move() { return side_to_move == BLACK; }

inline bool midgame() { return gamephase == MIDGAME; }
inline bool endgame() { return gamephase == ENDGAME; }
inline bool mopup  () { return gamephase == MOPUP;   }

template<Color Perspective>
uint8_t kingside_rights()
{
  constexpr Bitboard Mask = Perspective == WHITE ? 0b1000 : 0b0010;
  return st.castling_rights & Mask;
}
template<Color Perspective>
uint8_t queenside_rights()
{
  constexpr Bitboard Mask = Perspective == WHITE ? 0b0100 : 0b0001;
  return st.castling_rights & Mask;
}

inline Bitboard ep_bb() { return square_bb(st.ep_sq); }

inline uint64_t key() { return st.key; }

} // namespace Position

template<Piece P>
Bitboard bb() { return bitboards[P]; }

#define bb(P) bb<P>()

inline Piece piece_on(Square sq) { return board[sq]; }

inline PieceType piece_type_on(Square sq) { return type_of(board[sq]); }

inline Bitboard occupied_bb() { return bitboards[WHITE] | bitboards[BLACK]; }

template<Color> void do_move(Move m);
template<Color> void undo_move(Move m);

template<Color> ForceInline void do_capture(Move m);
template<Color> ForceInline void undo_capture(Move m);

void commit_move(Move m);

template<Color JustMoved, MoveType Type>
ForceInline void update_castling_rights() {

  constexpr Color Them = !JustMoved;

  constexpr uint8_t ClearRights = JustMoved == WHITE ? 0b0011 : 0b1100;

  constexpr Bitboard FriendlyKingStart = square_bb(JustMoved == WHITE ? E1 : E8);
  constexpr Bitboard EnemyKingStart    = square_bb(JustMoved == WHITE ? E8 : E1);
  constexpr Bitboard FriendlyRookStart = JustMoved == WHITE ? square_bb(A1, H1) : square_bb(A8, H8);

  constexpr Piece FriendlyKing = make_piece(JustMoved, KING);
  constexpr Piece FriendlyRook = make_piece(JustMoved, ROOK);
  constexpr Piece EnemyRook    = make_piece(Them     , ROOK);
  
  if constexpr (Type == NORMAL)
    Position::st.castling_rights &=
      castling_pext(bitboards[FriendlyKing] & FriendlyKingStart | bitboards[FriendlyRook] & FriendlyRookStart | bitboards[EnemyRook] | EnemyKingStart);

  else if constexpr (Type == PROMOTION)
    Position::st.castling_rights &= castling_pext(bitboards[EnemyRook] | FriendlyKingStart | FriendlyRookStart | EnemyKingStart);

  else if constexpr (Type == SHORTCASTLE || Type == LONGCASTLE)
    Position::st.castling_rights &= ClearRights;

}

template<Color Us>
ForceInline void do_capture(Move m) {

  constexpr Color Them  = !Us;
  constexpr Piece Pawn  = make_piece(Us, PAWN);
  constexpr Piece Queen = make_piece(Us, QUEEN);

  Position::side_to_move = !Position::side_to_move;

  Square from = from_sq(m);
  Square to   = to_sq(m);

  piece_stack.push(piece_on(to));

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
ForceInline void undo_capture(Move m) {

  constexpr Color Them  = !Us;
  constexpr Piece Pawn  = make_piece(Us, PAWN);
  constexpr Piece Queen = make_piece(Us, QUEEN);

  Position::side_to_move = !Position::side_to_move;

  Piece captured = piece_stack.pop();

  Square from = from_sq(m);
  Square to   = to_sq(m);

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

  using Position::st;

  constexpr Color Them = !Us;

  constexpr Piece Pawn  = make_piece(Us, PAWN);
  constexpr Piece Rook  = make_piece(Us, ROOK);
  constexpr Piece Queen = make_piece(Us, QUEEN);
  constexpr Piece King  = make_piece(Us, KING);

  constexpr Direction Up  = Us == WHITE ? NORTH : SOUTH;
  constexpr Direction Up2 = Up * 2;

  Position::side_to_move = !Position::side_to_move;

  Square from = from_sq(m);
  Square to   = to_sq(m);

  piece_stack.push(piece_on(to));
  st.ep_sq = 0;
  bool double_push = piece_type_on(to) == PAWN && to - from == Up2;

  Bitboard to_bb   = square_bb(to);
  Bitboard from_to = square_bb(from, to);

  switch (type_of(m)) {
  case NORMAL:
    st.ep_sq = (from + Up) * double_push;
    st.key ^= Zobrist::hash[board[from]][from];
    st.key ^= Zobrist::hash[board[from]][to];
    st.key ^= Zobrist::hash[board[to]][to];
    st.key ^= Zobrist::Side;
    bitboards[board[to]] &= ~to_bb;
    bitboards[Them] &= ~to_bb;
    bitboards[board[from]] ^= from_to;
    bitboards[Us] ^= from_to;
    board[to] = board[from];
    board[from] = NO_PIECE;
    update_castling_rights<Us, NORMAL>();
    return;
  case PROMOTION:
    st.key ^= Zobrist::hash[Pawn][from];
    st.key ^= Zobrist::hash[Queen][to];
    st.key ^= Zobrist::hash[board[to]][to];
    st.key ^= Zobrist::Side;
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

    st.key ^= Zobrist::hash[King][king_from];
    st.key ^= Zobrist::hash[King][king_to];
    st.key ^= Zobrist::hash[Rook][rook_from];
    st.key ^= Zobrist::hash[Rook][rook_to];
    st.key ^= Zobrist::Side;
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

    st.key ^= Zobrist::hash[King][king_from];
    st.key ^= Zobrist::hash[King][king_to];
    st.key ^= Zobrist::hash[Rook][rook_from];
    st.key ^= Zobrist::hash[Rook][rook_to];
    st.key ^= Zobrist::Side;
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
    st.key ^= Zobrist::hash[Pawn][from];
    st.key ^= Zobrist::hash[Pawn][to];
    st.key ^= Zobrist::hash[EPawn][capsq];
    st.key ^= Zobrist::Side;
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
void undo_move(Move m) {

  constexpr Color Them = !Us;

  constexpr Piece Pawn  = make_piece(Us, PAWN);
  constexpr Piece Rook  = make_piece(Us, ROOK);
  constexpr Piece Queen = make_piece(Us, QUEEN);
  constexpr Piece King  = make_piece(Us, KING);

  Position::side_to_move = !Position::side_to_move;

  Piece captured = piece_stack.pop();

  Square from = from_sq(m);
  Square to   = to_sq(m);

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
