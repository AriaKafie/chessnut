
#ifndef POSITION_H
#define POSITION_H

#include <cstddef>
#include <cstring>
#include <string>
#include <vector>

#include "bitboard.h"
#include "transpositiontable.h"
#include "types.h"

#define bb(P) bitboard<P>()

struct StateInfo
{
    uint64_t  key;
    uint8_t   castling_rights;
    Square    ep_sq;
    Piece     captured;
    Color     side_to_move;
    GamePhase gamephase;
};

extern Bitboard bitboards[16];
extern Piece board[SQUARE_NB];

extern StateInfo state_stack[MAX_PLY], *state_ptr;

namespace Zobrist
{
    constexpr uint64_t Side = 0xeeb3b2fe864d41e5ull;
    inline uint64_t hash[B_KING + 1][SQUARE_NB];
    inline uint64_t castling[1 << 4];
}

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
    constexpr Bitboard mask = JustMoved == WHITE ? square_bb(A1, E1, H1, A8, H8) : square_bb(A8, E8, H8, A1, H1);

    state_ptr->key ^= Zobrist::castling[state_ptr->castling_rights];
    state_ptr->castling_rights &= castle_masks[JustMoved][pext(bitboards[JustMoved], mask)];
    state_ptr->key ^= Zobrist::castling[state_ptr->castling_rights];
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

        bitboards[board[to]] &= zero_to;
        bitboards[Them] &= zero_to;
        bitboards[board[from]] ^= from_to;
        bitboards[Us] ^= from_to;

        board[to] = board[from];
        board[from] = NO_PIECE;

        update_castling_rights<Us>();

        RepetitionTable::push();

        return;
    case PROMOTION:
        state_ptr->key ^= Zobrist::hash[Pawn][from]
                       ^  Zobrist::hash[Queen][to]
                       ^  Zobrist::hash[board[to]][to]
                       ^  Zobrist::Side;

        bitboards[board[to]] &= zero_to;
        bitboards[Them] &= zero_to;
        bitboards[Pawn] ^= square_bb(from);
        bitboards[Queen] ^= ~zero_to;
        bitboards[Us] ^= from_to;

        board[to] = Queen;
        board[from] = NO_PIECE;
        
        update_castling_rights<Us>();

        RepetitionTable::push();

        return;
    case SHORTCASTLE:
    {
        constexpr Square king_from = Us == WHITE ? E1 : E8;
        constexpr Square king_to   = Us == WHITE ? G1 : G8;
        constexpr Square rook_from = Us == WHITE ? H1 : H8;
        constexpr Square rook_to   = Us == WHITE ? F1 : F8;

        constexpr Bitboard king_from_to = square_bb(king_from, king_to);
        constexpr Bitboard rook_from_to = square_bb(rook_from, rook_to);

        state_ptr->key ^= Zobrist::hash[King][king_from]
                       ^  Zobrist::hash[King][king_to]
                       ^  Zobrist::hash[Rook][rook_from]
                       ^  Zobrist::hash[Rook][rook_to]
                       ^  Zobrist::Side;

        bitboards[King] ^= king_from_to;
        bitboards[Rook] ^= rook_from_to;
        bitboards[Us] ^= king_from_to ^ rook_from_to;

        board[king_from] = NO_PIECE;
        board[rook_from] = NO_PIECE;
        board[king_to] = King;
        board[rook_to] = Rook;

        update_castling_rights<Us>();

        RepetitionTable::push();

        return;
    }
    case LONGCASTLE:
    {
        constexpr Square king_from = Us == WHITE ? E1 : E8;
        constexpr Square king_to   = Us == WHITE ? C1 : C8;
        constexpr Square rook_from = Us == WHITE ? A1 : A8;
        constexpr Square rook_to   = Us == WHITE ? D1 : D8;

        constexpr Bitboard king_from_to = square_bb(king_from, king_to);
        constexpr Bitboard rook_from_to = square_bb(rook_from, rook_to);

        state_ptr->key ^= Zobrist::hash[King][king_from]
                       ^  Zobrist::hash[King][king_to]
                       ^  Zobrist::hash[Rook][rook_from]
                       ^  Zobrist::hash[Rook][rook_to]
                       ^  Zobrist::Side;

        bitboards[King] ^= king_from_to;
        bitboards[Rook] ^= rook_from_to;
        bitboards[Us] ^= king_from_to ^ rook_from_to;

        board[king_from] = NO_PIECE;
        board[rook_from] = NO_PIECE;
        board[king_to] = King;
        board[rook_to] = Rook;

        update_castling_rights<Us>();

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

        bitboards[Pawn] ^= from_to;
        bitboards[EnemyPawn] ^= square_bb(capsq);
        bitboards[Us] ^= from_to;
        bitboards[Them] ^= square_bb(capsq);

        board[from] = NO_PIECE;
        board[to] = Pawn;
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
        return;
    }
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
        return;
    }
    case ENPASSANT:
        constexpr Piece EnemyPawn = make_piece(Them, PAWN);

        Square capsq = to + (Us == WHITE ? SOUTH : NORTH);

        bitboards[Pawn] ^= from_to;
        bitboards[Us] ^= from_to;
        bitboards[EnemyPawn] ^= square_bb(capsq);
        bitboards[Them] ^= square_bb(capsq);
        board[to] = NO_PIECE;
        board[from] = Pawn;
        board[capsq] = EnemyPawn;
        return;
    }
}

#endif
