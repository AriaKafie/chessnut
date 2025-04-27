
#ifndef MOVEORDERING_H
#define MOVEORDERING_H

#include <limits.h>

#include "evaluation.h"
#include "movelist.h"
#include "position.h"

/*template<Color Us>
int see(Square sq)
{
    constexpr Piece Pawn   = make_piece(Us, PAWN);
    constexpr Piece Knight = make_piece(Us, KNIGHT);
    constexpr Piece Bishop = make_piece(Us, BISHOP);
    constexpr Piece Rook   = make_piece(Us, ROOK);
    constexpr Piece Queen  = make_piece(Us, QUEEN);
    constexpr Piece King   = make_piece(Us, KING);

    int      value    = 0;
    Bitboard occupied = Position::occupied();
    Bitboard pawns    = bb(Pawn);
    Bitboard knights  = bb(Knight);
    Bitboard bishops  = bb(Bishop);
    Bitboard rooks    = bb(Rook);
    Bitboard queens   = bb(Queen);
    Bitboard king     = bb(King);

    Bitboard attackers =
        pawn_attacks<!Us>(sq)        & pawns
      | knight_attacks(sq)           & knights
      | bishop_attacks(sq, occupied) & (bishops | queens)
      | rook_attacks(sq, occupied)   & (rooks | queens)
      | king_attacks(sq)             & king;

    Square smallest_attacker =
            attackers & pawns   ? lsb(pawns)
        :   attackers & knights ? lsb(knights)
        :   attackers & bishops ? lsb(bishops)
        :   attackers & rooks   ? lsb(rooks)
        :   attackers & queens  ? lsb(queens)
        :   attackers & king    ? lsb(king) : NO_SQ;

    if (smallest_attacker != NO_SQ)
    {
        Square   from     = smallest_attacker, to = sq;
        Bitboard from_to  = square_bb(from, to), to_bb = square_bb(to);
        Piece    attacker = piece_on(from);
        Piece    victim   = piece_on(to);

            bitboards[victim]   ^= to_bb;
            bitboards[!Us]      ^= to_bb;
            bitboards[attacker] ^= from_to;
            bitboards[Us]       ^= from_to;
            board[to]            = attacker;
            board[from]          = NO_PIECE;

        value = std::max(0, piece_weight(piece_type_of(victim)) - see<!Us>(sq));

            bitboards[attacker] ^= from_to;
            bitboards[Us]       ^= from_to;
            bitboards[victim]   ^= to_bb;
            bitboards[!Us]      ^= to_bb;
            board[from]          = attacker;
            board[to]            = victim;
    }

    return value;
}

template<Color Us>
int see_capture(Move m)
{
    Square   from     = from_sq(m), to = to_sq(m);
    Bitboard from_to  = square_bb(from, to), to_bb = square_bb(to);
    Piece    attacker = piece_on(from);
    Piece    victim   = piece_on(to);

        bitboards[victim]   ^= to_bb;
        bitboards[!Us]      ^= to_bb;
        bitboards[attacker] ^= from_to;
        bitboards[Us]       ^= from_to;
        board[to]            = attacker;
        board[from]          = NO_PIECE;

    int value = piece_weight(piece_type_of(victim)) - see<!Us>(to);

        bitboards[attacker] ^= from_to;
        bitboards[Us]       ^= from_to;
        bitboards[victim]   ^= to_bb;
        bitboards[!Us]      ^= to_bb;
        board[from]          = attacker;
        board[to]            = victim;

    return value;
}*/

constexpr Score BAD_QUIET_MIN      = (UINT16_MAX / 4) * 0;
constexpr Score BAD_CAPTURE_MIN    = (UINT16_MAX / 4) * 1;
constexpr Score GOOD_QUIET_MIN     = (UINT16_MAX / 4) * 2;
constexpr Score GOOD_CAPTURE_MIN   = (UINT16_MAX / 4) * 3;

constexpr Score BAD_QUIET_BASE     = BAD_QUIET_MIN    + (UINT16_MAX / 8);
constexpr Score BAD_CAPTURE_BASE   = BAD_CAPTURE_MIN  + (UINT16_MAX / 8);
constexpr Score GOOD_QUIET_BASE    = GOOD_QUIET_MIN   + (UINT16_MAX / 8);
constexpr Score GOOD_CAPTURE_BASE  = GOOD_CAPTURE_MIN + (UINT16_MAX / 8);

constexpr Score MAX_SCORE          = UINT16_MAX;
constexpr Score SEEN_BY_PAWN_MALUS = 50;

template<Color Us>
void CaptureList<Us>::insertion_sort()
{
    for (int i = 1; i < size(); i++)
    {
        LMove key = moves[i];

        int j = i - 1;

        while (j >= 0 && score_of(moves[j]) < score_of(key))
        {
            moves[j + 1] = moves[j];
            j--;
        }

        moves[j + 1] = key;
    }
}

template<Color Us>
void CaptureList<Us>::sort()
{
    Bitboard seen_by_pawn = pawn_attacks<!Us>(bitboard<make_piece(!Us, PAWN)>());

    for (LMove& m : *this)
    {  
        Score score = UINT16_MAX / 2;

        Square    from     = from_sq(m);
        Square    to       = to_sq(m);
        PieceType pt       = piece_type_on(from);
        PieceType captured = piece_type_on(to);

        if (square_bb(to) & seen_by_pawn)
            score -= 500;

        score += piece_weight(captured) - piece_weight(pt) * bool(square_bb(to) & seen_by_enemy);

        set_score(m, score);
    }
  
    insertion_sort();
}

template<Color Us>
int MoveList<Us>::partition(int low, int high)
{
    Score pivot = score_of(moves[high]);

    int i = low - 1;

    for (int j = low; j < high; j++)
    {
        if (score_of(moves[j]) >= pivot)
        {
            i++;
            std::swap(moves[i], moves[j]);
        }
    }

    std::swap(moves[i + 1], moves[high]);

    return i + 1;
}

template<Color Us>
void MoveList<Us>::quicksort(int low, int high) {
    if (low < high) {
        int pivot_index = partition(low, high);
        quicksort(low, pivot_index - 1);
        quicksort(pivot_index + 1, high);
    }
}

template<Color Us>
void MoveList<Us>::sort(Move ttmove, SearchInfo *si)
{
    Bitboard seen_by_pawn = pawn_attacks<!Us>(bitboard<make_piece(!Us, PAWN)>());

    for (LMove& m : *this)
    {
        if (m == ttmove)
        {
            set_score(m, MAX_SCORE);
            continue;
        }
        
        Score score;
    
        Square    from     = from_sq(m);
        Square    to       = to_sq(m);
        PieceType pt       = piece_type_on(from);
        PieceType captured = piece_type_on(to);

        if (captured)
        {
            /*int see_score = see_capture<Us>(m);

            score = see_score >= 0 ? GOOD_CAPTURE_BASE : BAD_CAPTURE_BASE;
            score += see_score;*/

            int material_delta = piece_weight(captured) - piece_weight(pt);

            if (square_bb(to) & seen_by_enemy)
                score = (material_delta >= 0 ? GOOD_CAPTURE_BASE : BAD_CAPTURE_BASE) + material_delta;
            else
                score = GOOD_CAPTURE_BASE + material_delta;
        }
        else
        {
            if (m == si->killers[0]
             || m == si->killers[1])
                score = GOOD_QUIET_BASE;
            else
                score = BAD_QUIET_BASE;

            score += Position::endgame() && pt == KING 
                ? (end_king_squares[to] - end_king_squares[from]) / 2
                : (square_score<Us>(pt, to) - square_score<Us>(pt, from)) / 2;
        }

        if (type_of(m) == PROMOTION)
            score += promotion_type_of(m);

        if (square_bb(to) & seen_by_pawn)
            score -= SEEN_BY_PAWN_MALUS;

        set_score(m, score);
    }
    
    quicksort(0, size() - 1);
}

#endif