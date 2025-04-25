
#ifndef MOVEORDERING_H
#define MOVEORDERING_H

#include "evaluation.h"
#include "movelist.h"
#include "position.h"

constexpr int MAX_SCORE          = 0x7fffffff;
constexpr int GOOD_CAPTURE_BONUS = 8000;
constexpr int BAD_CAPTURE_BONUS  = 2000;
constexpr int GOOD_QUIET_BONUS   = 4000;
constexpr int SEEN_BY_PAWN_MALUS = 50;
constexpr int PROMOTION_BONUS    = 50;

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
        int score = 0;

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
    int pivot = score_of(moves[high]);

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
        
        int score = 0;
    
        Square    from     = from_sq(m);
        Square    to       = to_sq(m);
        PieceType pt       = piece_type_on(from);
        PieceType captured = piece_type_on(to);

        if (captured)
        {
            int material_delta = piece_weight(captured) - piece_weight(pt);

            if (square_bb(to) & seen_by_enemy)
                score += (material_delta >= 0 ? GOOD_CAPTURE_BONUS : BAD_CAPTURE_BONUS) + material_delta;
            else
                score += GOOD_CAPTURE_BONUS + material_delta;
        }
        else
        {
            if (m == si->killers[0]
             || m == si->killers[1])
                score += GOOD_QUIET_BONUS;

            score += Position::endgame() && pt == KING ? (end_king_squares[to] - end_king_squares[from]) / 2 : (square_score<Us>(pt, to) - square_score<Us>(pt, from)) / 2;
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