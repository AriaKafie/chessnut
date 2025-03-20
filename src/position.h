
#ifndef POSITION_H
#define POSITION_H

#include <cstring>
#include <string>
#include <vector>

#include "bitboard.h"
#include "transpositiontable.h"
#include "types.h"

#define bb(P) bitboard<P>()

typedef struct
{
    uint64_t  key;
    uint8_t   castling_rights;
    Square    ep_sq;
    Piece     captured;
    Color     side_to_move;
    GamePhase gamephase;
} StateInfo;

extern Bitboard bitboards[16];
extern Piece board[SQUARE_NB];

extern StateInfo state_stack[MAX_PLY], *state_ptr;

namespace Zobrist { constexpr uint64_t Side = 0xeeb3b2fe864d41e5ull; inline uint64_t hash[B_KING + 1][SQUARE_NB]; }

template<Piece P>
Bitboard bitboard() { return bitboards[P]; }

namespace Position {

void init();
void set(const std::string& fen);
void commit_move(Move m);
std::string fen();
std::string to_string();

inline bool white_to_move() { return state_ptr->side_to_move == WHITE; }
inline bool black_to_move() { return state_ptr->side_to_move == BLACK; }

inline bool midgame() { return state_ptr->gamephase == MIDGAME; }
inline bool endgame() { return state_ptr->gamephase == ENDGAME; }
inline bool mopup  () { return state_ptr->gamephase == MOPUP;   }

inline Bitboard occupied() { return bitboards[WHITE] | bitboards[BLACK]; }

inline Bitboard ep_bb() { return square_bb(state_ptr->ep_sq); }

inline uint64_t key() { return state_ptr->key; }

inline Color side_to_move() { return state_ptr->side_to_move; }

template<Color SideToMove>
bool in_check()
{
    constexpr Color Them = !SideToMove;

    constexpr Piece EnemyPawn   = make_piece(Them, PAWN);
    constexpr Piece EnemyKnight = make_piece(Them, KNIGHT);
    constexpr Piece EnemyBishop = make_piece(Them, BISHOP);
    constexpr Piece EnemyRook   = make_piece(Them, ROOK);
    constexpr Piece EnemyQueen  = make_piece(Them, QUEEN);

    Square ksq = lsb(bitboard<make_piece(SideToMove, KING)>());

    return
        pawn_attacks<SideToMove>(ksq)   &  bb(EnemyPawn)
      | knight_attacks(ksq)             &  bb(EnemyKnight)
      | bishop_attacks(ksq, occupied()) & (bb(EnemyQueen) | bb(EnemyBishop))
      | rook_attacks  (ksq, occupied()) & (bb(EnemyQueen) | bb(EnemyRook));
}

} // namespace Position

inline Piece piece_on(Square sq) { return board[sq]; }

inline bool is_capture(Move m) { return piece_on(to_sq(m)); }

inline bool is_quiet(Move m) { return !piece_on(to_sq(m)); }

inline PieceType piece_type_on(Square sq) { return type_of(board[sq]); }

template<Color JustMoved>
ForceInline void update_castling_rights()
{
    constexpr Bitboard Mask = JustMoved == WHITE ? square_bb(A1, E1, H1, A8, H8) : square_bb(A8, E8, H8, A1, H1);
#ifdef BMI
    state_ptr->castling_rights &= CastleMasks[JustMoved][pext(bitboards[JustMoved], Mask)];
#else
    constexpr Bitboard Magic = JustMoved == WHITE ? 0x4860104020003061ull : 0x1080000400400c21ull;
    state_ptr->castling_rights &= CastleMasks[JustMoved][(bitboards[JustMoved] & Mask) * Magic >> 59];
#endif
}

template<Color Us>
ForceInline void do_capture(Move m)
{
    constexpr Color Them  = !Us;
    constexpr Piece Pawn  = make_piece(Us, PAWN);
    constexpr Piece Queen = make_piece(Us, QUEEN);

    Square from = from_sq(m), to = to_sq(m);

    Bitboard to_bb   = square_bb(to);
    Bitboard from_to = square_bb(from, to);

    switch (type_of(m))
    {
    case NORMAL:
        bitboards[board[to]]   ^= to_bb;
        bitboards[Them]        ^= to_bb;
        bitboards[board[from]] ^= from_to;
        bitboards[Us]          ^= from_to;

        board[to]   = board[from];
        board[from] = NO_PIECE;

        return;
    case PROMOTION:
        Piece promotion = make_piece(Us, promotion_type_of(m));

        bitboards[board[to]] ^= to_bb;
        bitboards[Them]      ^= to_bb;
        bitboards[Pawn]      ^= square_bb(from);
        bitboards[promotion] ^= to_bb;
        bitboards[Us]        ^= from_to;

        board[to]   = promotion;
        board[from] = NO_PIECE;

        return;
    }
}

template<Color Us>
ForceInline void undo_capture(Move m, Piece captured)
{
    constexpr Color Them  = !Us;
    constexpr Piece Pawn  = make_piece(Us, PAWN);
    constexpr Piece Queen = make_piece(Us, QUEEN);

    Square from = from_sq(m), to = to_sq(m);

    Bitboard to_bb   = square_bb(to);
    Bitboard from_to = square_bb(from, to);

    switch (type_of(m))
    {
    case NORMAL:
        bitboards[board[to]] ^= from_to;
        bitboards[Us]        ^= from_to;
        bitboards[captured]  ^= to_bb;
        bitboards[Them]      ^= to_bb;

        board[from] = board[to];
        board[to]   = captured;

        return;
    case PROMOTION:
        bitboards[board[to]] ^= to_bb;
        bitboards[Pawn]      ^= square_bb(from);
        bitboards[Us]        ^= from_to;
        bitboards[captured]  ^= to_bb;
        bitboards[Them]      ^= to_bb;

        board[from] = Pawn;
        board[to]   = captured;

        return;
    }
}

template<Color Us>
void do_move(Move m)
{
    constexpr Color Them = !Us;

    constexpr Piece Pawn  = make_piece(Us, PAWN);
    constexpr Piece Rook  = make_piece(Us, ROOK);
    constexpr Piece Queen = make_piece(Us, QUEEN);
    constexpr Piece King  = make_piece(Us, KING);

    constexpr Direction Up = Us == WHITE ? NORTH : SOUTH;

    Square from = from_sq(m), to = to_sq(m);

    memcpy(state_ptr + 1, state_ptr, sizeof(StateInfo));
    state_ptr++;
    state_ptr->captured = piece_on(to);
    state_ptr->ep_sq = (from + Up) * !(from ^ to ^ 16 | piece_on(from) ^ Pawn);

    Bitboard zero_to = ~square_bb(to);
    Bitboard from_to =  square_bb(from, to);

    switch (type_of(m))
    {
    case NORMAL:
        state_ptr->key ^= Zobrist::hash[board[from]][from]
                       ^  Zobrist::hash[board[from]][to]
                       ^  Zobrist::hash[board[to]][to]
                       ^  Zobrist::Side;

        bitboards[board[to]]   &= zero_to;
        bitboards[Them]        &= zero_to;
        bitboards[board[from]] ^= from_to;
        bitboards[Us]          ^= from_to;

        board[to]   = board[from];
        board[from] = NO_PIECE;

        update_castling_rights<Us>();

        RepetitionTable::push();

        return;
    case PROMOTION:
    {
        Piece promotion = make_piece(Us, promotion_type_of(m));

        state_ptr->key ^= Zobrist::hash[Pawn][from]
                       ^  Zobrist::hash[promotion][to]
                       ^  Zobrist::hash[board[to]][to]
                       ^  Zobrist::Side;

        bitboards[board[to]] &= zero_to;
        bitboards[Them]      &= zero_to;
        bitboards[Pawn]      ^= square_bb(from);
        bitboards[promotion] ^= ~zero_to;
        bitboards[Us]        ^= from_to;

        board[to]   = promotion;
        board[from] = NO_PIECE;
        
        update_castling_rights<Us>();

        RepetitionTable::push();

        return;
    }
    case CASTLING:
    {
        Move rook_move = Us == WHITE ? to == G1 ? make_move(H1, F1)
                                                : make_move(A1, D1)
                                     : to == G8 ? make_move(H8, F8)
                                                : make_move(A8, D8);

        Square   rfrom    = from_sq(rook_move);
        Square   rto      = to_sq(rook_move);
        Bitboard rfrom_to = square_bb(rfrom, rto);

        state_ptr->key ^= Zobrist::hash[King][from]
                       ^  Zobrist::hash[King][to]
                       ^  Zobrist::hash[Rook][rfrom]
                       ^  Zobrist::hash[Rook][rto]
                       ^  Zobrist::Side;

        bitboards[King] ^= from_to;
        bitboards[Rook] ^= rfrom_to;
        bitboards[Us]   ^= from_to ^ rfrom_to;

        board[from]  = NO_PIECE;
        board[rfrom] = NO_PIECE;
        board[to]    = King;
        board[rto]   = Rook;

        constexpr uint8_t mask = Us == WHITE ? 0b0011 : 0b1100;
        state_ptr->castling_rights &= mask;

        RepetitionTable::push();

        return;
    }
    case ENPASSANT:
        constexpr Piece EnemyPawn = make_piece(Them, PAWN);

        Square capsq = to + (Us == WHITE ? SOUTH : NORTH);

        state_ptr->key ^= Zobrist::hash[Pawn][from]
                       ^  Zobrist::hash[Pawn][to]
                       ^  Zobrist::hash[EnemyPawn][capsq]
                       ^  Zobrist::Side;

        bitboards[Pawn]      ^= from_to;
        bitboards[EnemyPawn] ^= square_bb(capsq);
        bitboards[Us]        ^= from_to;
        bitboards[Them]      ^= square_bb(capsq);

        board[from]  = NO_PIECE;
        board[to]    = Pawn;
        board[capsq] = NO_PIECE;
        
        RepetitionTable::push();

        return;
    }
}

template<Color Us>
void undo_move(Move m)
{
    RepetitionTable::pop();

    constexpr Color Them = !Us;

    constexpr Piece Pawn  = make_piece(Us, PAWN);
    constexpr Piece Rook  = make_piece(Us, ROOK);
    constexpr Piece Queen = make_piece(Us, QUEEN);
    constexpr Piece King  = make_piece(Us, KING);

    Piece captured = state_ptr--->captured;

    Square from = from_sq(m), to = to_sq(m);

    Bitboard to_bb      = square_bb(to);
    Bitboard from_to    = square_bb(from, to);
    Bitboard capture_bb = to_bb * bool(captured);

    switch (type_of(m))
    {
    case NORMAL:
        bitboards[board[to]] ^= from_to;
        bitboards[Us]        ^= from_to;
        bitboards[captured]  ^= capture_bb;
        bitboards[Them]      ^= capture_bb;

        board[from] = board[to];
        board[to]   = captured;

        return;
    case PROMOTION:
        bitboards[board[to]] ^= to_bb;
        bitboards[Pawn]      ^= square_bb(from);
        bitboards[Us]        ^= from_to;
        bitboards[captured]  ^= capture_bb;
        bitboards[Them]      ^= capture_bb;

        board[to]   = captured;
        board[from] = Pawn;

        return;
    case CASTLING:
    {
        Move rook_move = Us == WHITE ? to == G1 ? make_move(H1, F1)
                                                : make_move(A1, D1)
                                     : to == G8 ? make_move(H8, F8)
                                                : make_move(A8, D8);

        Square   rfrom    = from_sq(rook_move), rto = to_sq(rook_move);
        Bitboard rfrom_to = square_bb(rfrom, rto);

        bitboards[King] ^= from_to;
        bitboards[Rook] ^= rfrom_to;
        bitboards[Us]   ^= from_to ^ rfrom_to;

        board[to]    = NO_PIECE;
        board[rto]   = NO_PIECE;
        board[from]  = King;
        board[rfrom] = Rook;

        return;
    }
    case ENPASSANT:
        constexpr Piece EnemyPawn = make_piece(Them, PAWN);

        Square capsq = to + (Us == WHITE ? SOUTH : NORTH);

        bitboards[Pawn]      ^= from_to;
        bitboards[Us]        ^= from_to;
        bitboards[EnemyPawn] ^= square_bb(capsq);
        bitboards[Them]      ^= square_bb(capsq);

        board[to]    = NO_PIECE;
        board[from]  = Pawn;
        board[capsq] = EnemyPawn;

        return;
    }
}

#endif
