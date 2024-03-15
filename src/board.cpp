
#include "board.h"
#include "zobrist.h"

Bitboard bitboards[COLOR_NB + PIECE_TYPE_NB];
Piece board[SQUARE_NB];

template<Color Us>
ForceInline void do_capture(Move m) {

  constexpr Color Them  = !Us;
  constexpr Piece Pawn  = make_piece(Us, PAWN);
  constexpr Piece Queen = make_piece(Us, QUEEN);

  Square from = from_sq(m);
  Square to   = to_sq(m);

  Bitboard from_bb = square_bb(from);
  Bitboard to_bb   = square_bb(to);
  Bitboard from_to = from_bb | to_bb;

  switch (type_of(m)) {
  case NORMAL:
    bitboards[piece_type_on(to)] ^= to_bb;
    bitboards[Them] ^= to_bb;
    bitboards[piece_type_on(from)] ^= from_to;
    bitboards[Us] ^= from_to;
    board[to] = board[from];
    board[from] = NO_PIECE;
    return;
  case PROMOTION:
    bitboards[piece_type_on(to)] ^= to_bb;
    bitboards[Them] ^= to_bb;
    bitboards[Pawn] ^= from_bb;
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

  Square from = from_sq(m);
  Square to   = to_sq(m);

  Bitboard from_bb = square_bb(from);
  Bitboard to_bb   = square_bb(to);
  Bitboard from_to = to_bb | square_bb(from);

  switch (type_of(m)) {
  case NORMAL:
    bitboards[piece_type_on(to)] ^= from_to;
    bitboards[type_of(captured)] ^= to_bb;
    bitboards[Us] ^= from_to;
    bitboards[Them] ^= to_bb;
    board[from] = board[to];
    board[to] = captured;
    return;
  case PROMOTION:
    bitboards[Queen] ^= to_bb;
    bitboards[Pawn] ^= from_bb;
    bitboards[Us] ^= from_to;
    bitboards[type_of(captured)] ^= to_bb;
    bitboards[Them] ^= to_bb;
    board[from] = Pawn;
    board[to] = captured;
    return;
  }
}

template<Color Us>
ForceInline void do_move(Move m) {
  
  constexpr Color Them = !Us;

  constexpr Piece Pawn  = make_piece(Us, PAWN);
  constexpr Piece Rook  = make_piece(Us, ROOK);
  constexpr Piece Queen = make_piece(Us, QUEEN);
  constexpr Piece King  = make_piece(Us, KING);

  Square from = from_sq(m);
  Square to   = to_sq(m);

  Bitboard from_bb = square_bb(from);
  Bitboard to_bb   = square_bb(to);
  Bitboard from_to = from_bb | to_bb;

  switch (type_of(m)) {
  case NORMAL:
    Zobrist::key ^= Zobrist::hash[piece_on(from)][from];
    Zobrist::key ^= Zobrist::hash[piece_on(from)][to];
    Zobrist::key ^= Zobrist::hash[piece_on(to)][to];
    Zobrist::key ^= Zobrist::BlackToMove;
    and_not(bitboards[piece_type_on(to)], to_bb);
    and_not(bitboards[Them], to_bb);
    bitboards[piece_type_on(from)] ^= from_to;
    bitboards[Us] ^= from_to;
    board[to] = board[from];
    board[from] = NO_PIECE;
    update_castling_rights<Us, NORMAL>();
    return;
  case PROMOTION:
    Zobrist::key ^= Zobrist::hash[Pawn][from];
    Zobrist::key ^= Zobrist::hash[Queen][to];
    Zobrist::key ^= Zobrist::hash[piece_on(to)][to];
    Zobrist::key ^= Zobrist::BlackToMove;
    and_not(bitboards[piece_type_on(to)], to_bb);
    and_not(bitboards[Them], to_bb);
    bitboards[PAWN] ^= from_bb;
    bitboards[QUEEN] ^= to_bb;
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
    bitboards[KING] ^= king_from_to;
    bitboards[ROOK] ^= rook_from_to;
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
    bitboards[KING] ^= king_from_to;
    bitboards[ROOK] ^= rook_from_to;
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
    bitboards[PAWN] ^= from_to ^ square_bb(capsq);
    bitboards[Us] ^= from_to;
    bitboards[Them] ^= square_bb(capsq);
    board[from] = NO_PIECE;
    board[to] = Pawn;
    board[capsq] = NO_PIECE;
    return;
  }
}

template<Color Us>
ForceInline void undo_move(Move m, Piece captured) {
  
  constexpr Color Them = !Us;

  constexpr Piece Pawn  = make_piece(Us, PAWN);
  constexpr Piece Rook  = make_piece(Us, ROOK);
  constexpr Piece Queen = make_piece(Us, QUEEN);
  constexpr Piece King  = make_piece(Us, KING);

  Square from = from_sq(m);
  Square to   = to_sq(m);

  Bitboard from_bb    = square_bb(from);
  Bitboard to_bb      = square_bb(to);
  Bitboard from_to    = from_bb | to_bb;
  Bitboard capture_bb = to_bb * bool(captured);

  switch (type_of(m)) {
  case NORMAL:
    Zobrist::key ^= Zobrist::hash[piece_on(to)][to];
    Zobrist::key ^= Zobrist::hash[piece_on(to)][from];
    Zobrist::key ^= Zobrist::hash[captured][to];
    Zobrist::key ^= Zobrist::BlackToMove;
    bitboards[piece_type_on(to)] ^= from_to;
    bitboards[Us] ^= from_to;
    bitboards[type_of(captured)] ^= capture_bb;
    bitboards[Them] ^= capture_bb;
    board[from] = board[to];
    board[to] = captured;
    return;
  case PROMOTION:
    Zobrist::key ^= Zobrist::hash[Queen][to];
    Zobrist::key ^= Zobrist::hash[Pawn][from];
    Zobrist::key ^= Zobrist::hash[captured][to];
    Zobrist::key ^= Zobrist::BlackToMove;
    bitboards[QUEEN] ^= to_bb;
    bitboards[PAWN] ^= from_bb;
    bitboards[Us] ^= from_to;
    bitboards[type_of(captured)] ^= capture_bb;
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

    Zobrist::key ^= Zobrist::hash[King][king_to];
    Zobrist::key ^= Zobrist::hash[King][king_from];
    Zobrist::key ^= Zobrist::hash[Rook][rook_to];
    Zobrist::key ^= Zobrist::hash[Rook][rook_from];
    Zobrist::key ^= Zobrist::BlackToMove;
    bitboards[KING] ^= king_from_to;
    bitboards[ROOK] ^= rook_from_to;
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

    Zobrist::key ^= Zobrist::hash[King][king_to];
    Zobrist::key ^= Zobrist::hash[King][king_from];
    Zobrist::key ^= Zobrist::hash[Rook][rook_to];
    Zobrist::key ^= Zobrist::hash[Rook][rook_from];
    Zobrist::key ^= Zobrist::BlackToMove;
    bitboards[KING] ^= king_from_to;
    bitboards[ROOK] ^= rook_from_to;
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
    Zobrist::key ^= Zobrist::hash[Pawn][to];
    Zobrist::key ^= Zobrist::hash[Pawn][from];
    Zobrist::key ^= Zobrist::hash[EPawn][capsq];
    Zobrist::key ^= Zobrist::BlackToMove;
    bitboards[PAWN] ^= from_to ^ square_bb(capsq);
    bitboards[Us] ^= from_to;
    bitboards[Them] ^= square_bb(capsq);
    board[to] = NO_PIECE;
    board[from] = Pawn;
    board[capsq] = EPawn;
    return;
  }
}
