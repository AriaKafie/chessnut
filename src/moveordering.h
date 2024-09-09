
#ifndef MOVEORDERING_H
#define MOVEORDERING_H

#include "evaluation.h"
#include "movelist.h"
#include "position.h"
#include "search.h"

struct Killer {

    Move moveA;
    Move moveB;

    void add(Move m) {

        if (m != moveA)
        {
            moveB = moveA;
            moveA = m;
        }
    }

    bool contains(Move m) const {
        return (m == moveA) || (m == moveB);
    }

};

inline Killer killer_moves[MAX_PLY];

constexpr uint32_t MAX_SCORE          = 0xffff0000;
constexpr uint32_t GOOD_CAPTURE_BONUS = 8000;
constexpr uint32_t BAD_CAPTURE_BONUS  = 2000;
constexpr uint32_t EVASION_BONUS      = 1000;
constexpr uint32_t KILLER_BONUS       = 4000;
constexpr uint32_t SEEN_BY_PAWN_MALUS = 50;
constexpr uint32_t PROMOTION_BONUS    = 50;
constexpr uint32_t GIVES_CHECK_BONUS  = 200;

template<Color Us>
void CaptureList<Us>::insertion_sort() {

    for (int i = 1; i < size(); i++)
    {
        Move key = moves[i];

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
void CaptureList<Us>::sort() {

    Bitboard seen_by_pawn = pawn_attacks<!Us>(bitboard<make_piece(!Us, PAWN)>());
            
    for (Move& m : *this)
    {  
        uint32_t score = 0x7fff;

        Square    from     = from_sq(m);
        Square    to       = to_sq(m);
        PieceType pt       = piece_type_on(from);
        PieceType captured = piece_type_on(to);

        if (square_bb(to) & seen_by_pawn)
            score -= 500;

        score += piece_weight(captured) - piece_weight(pt) * bool(square_bb(to) & seen_by_enemy);

        m += score << 16;
    }
  
    insertion_sort();
}

template<Color Us>
int MoveList<Us>::partition(int low, int high) {

    uint32_t pivot = score_of(moves[high]);

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

  if (low < high)
  {
    int pivot_index = partition(low, high);
    quicksort(low, pivot_index - 1);
    quicksort(pivot_index + 1, high);
  }
}

template<Color Us>
void MoveList<Us>::sort(Move best_move, int ply) {

    Bitboard seen_by_pawn = pawn_attacks<!Us>(bitboard<make_piece(!Us, PAWN)>());

    for (Move& m : *this)
    {
        if (m == (best_move & 0xffff))
        {
            m += MAX_SCORE;
            continue;
        }
      
        uint32_t score = 0x7fff;
    
        Square    from     = from_sq(m);
        Square    to       = to_sq(m);
        PieceType pt       = piece_type_on(from);
        PieceType captured = piece_type_on(to);

        Bitboard enemy_king = bitboard<make_piece(!Us, KING)>();

        if
        (
            pt == PAWN   && (pawn_attacks<WHITE>(to)           & enemy_king)
        ||  pt == KNIGHT && (knight_attacks(to)                & enemy_king)
        ||  pt == BISHOP && (bishop_attacks(to, occupied_bb()) & enemy_king)
        ||  pt == ROOK   && (rook_attacks  (to, occupied_bb()) & enemy_king)
        ||  pt == QUEEN  && (queen_attacks (to, occupied_bb()) & enemy_king)
        )
            score += GIVES_CHECK_BONUS;

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
            if (killer_moves[ply].contains(m))
                score += KILLER_BONUS;

            score += Position::endgame() && pt == KING ? (end_king_squares[to] - end_king_squares[from]) / 2 : (square_score<Us>(pt, to) - square_score<Us>(pt, from)) / 2;
        }
        
        if (type_of(m) == PROMOTION)
            score += PROMOTION_BONUS;

        if (square_bb(to) & seen_by_pawn)
            score -= SEEN_BY_PAWN_MALUS;

        m += score << 16;
    }
    
    quicksort(0, size() - 1);
}

#endif
