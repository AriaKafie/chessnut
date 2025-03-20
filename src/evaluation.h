
#ifndef EVALUATION_H
#define EVALUATION_H

#include "position.h"
#include "types.h"

constexpr int piece_weights[KING + 1] = { 0, 0, 100, 300, 300, 500, 900, 1500 };

inline int piece_weight(PieceType pt) { return piece_weights[pt]; }

constexpr int square_scores[PIECE_TYPE_NB][SQUARE_NB] = 
{
// scored from black's pov (promotion = 0-7) with a maximizer perspective
    { // pawn
        50, 50, 50, 50, 50, 50, 50, 50,
        50, 50, 50, 50, 50, 50, 50, 50,
        10, 10, 20, 30, 30, 20, 10, 10,
        5,  5, 10, 25, 25, 10,  5,  5,
        0,  0,  0, 20, 20,  0,  0,  0,
        5, -5,-10,  0,  0,-10, -5,  5,
        5, 10, 10,-20,-20, 10, 10,  5,
        0,  0,  0,  0,  0,  0,  0,  0
    },
    { // knight
        -50,-40,-30,-30,-30,-30,-40,-50,
        -40,-20,  0,  0,  0,  0,-20,-40,
        -30,  0, 10, 15, 15, 10,  0,-30,
        -30,  5, 15, 20, 20, 15,  5,-30,
        -30,  0, 15, 20, 20, 15,  0,-30,
        -30,  5, 10, 15, 15, 10,  5,-30,
        -40,-20,  0,  5,  5,  0,-20,-40,
        -50,-40,-30,-30,-30,-30,-40,-50,
    },
    { // bishop
        -20,-10,-10,-10,-10,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5, 10, 10,  5,  0,-10,
        -10,  5,  5, 10, 10,  5,  5,-10,
        -10,  0, 10, 10, 10, 10,  0,-10,
        -10, 10, 10, 10, 10, 10, 10,-10,
        -10,  5,  0,  0,  0,  0,  5,-10,
        -20,-10,-10,-10,-10,-10,-10,-20,
    },
    { // rook
        0,  0,  0,  0,  0,  0,  0,  0,
        5, 10, 10, 10, 10, 10, 10,  5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  5,  5,  0,  0, -5
    },
    { // queen
        -20,-10,-10, -5, -5,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5,  5,  5,  5,  0,-10,
        -5,  0,  5,  5,  5,  5,  0, -5,
        0,  0,  5,  5,  5,  5,  0, -5,
        -10,  5,  5,  5,  5,  5,  0,-10,
        -10,  0,  5,  0,  0,  0,  0,-10,
        -20,-10,-10, -5, -5,-10,-10,-20
    },
    { // king
        -6,-8,-8,-10,-10,-8,-8,-6,
        -6,-8,-8,-10,-10,-8,-8,-6,
        -6,-8,-8,-10,-10,-8,-8,-6,
        -6,-8,-8,-10,-10,-8,-8,-6,
        -4,-6,-6,-8, -8, -6,-6,-4,
        -2,-4,-4,-4, -4, -4,-4,-2,
        -2,-2,-1,-1, -1, -1,-2,-2,
        0, 0, 0, 0,  0,  0, 0, 0,
    }
};

template<Color Perspective>
constexpr int square_score(PieceType pt, Square sq)
{
    if constexpr (Perspective == WHITE) return square_scores[pt - 2][sq ^ 63];
    else                                return square_scores[pt - 2][sq     ];
}

constexpr int end_king_squares[] =
{
    -10,-8,-6,-4,-4,-6,-8,-10,
    -6, -4,-2, 0, 0,-2,-4,-6,
    -6, -2, 4, 6, 6, 4,-2,-6,
    -6, -2, 6, 8, 8, 6,-2,-6,
    -6, -2, 6, 8, 8, 6,-2,-6,
    -6, -2, 4, 6, 6, 4,-2,-6,
    -6, -6, 0, 0, 0, 0,-6,-6,
    -10,-6,-6,-6,-6,-6,-6,-10,
};

template<Color Us>
int material_count()
{
    return
        100 * (popcount(bitboard<make_piece(Us, PAWN  )>()) - popcount(bitboard<make_piece(!Us, PAWN  )>())) +
        300 * (popcount(bitboard<make_piece(Us, KNIGHT)>()) - popcount(bitboard<make_piece(!Us, KNIGHT)>())) +
        300 * (popcount(bitboard<make_piece(Us, BISHOP)>()) - popcount(bitboard<make_piece(!Us, BISHOP)>())) +
        500 * (popcount(bitboard<make_piece(Us, ROOK  )>()) - popcount(bitboard<make_piece(!Us, ROOK  )>())) +
        900 * (popcount(bitboard<make_piece(Us, QUEEN )>()) - popcount(bitboard<make_piece(!Us, QUEEN )>()));
}

template<Color Us>
int midgame()
{
    int score = material_count<Us>();

    constexpr Color Them         = !Us;
    constexpr Piece FriendlyPawn = make_piece(Us,   PAWN);
    constexpr Piece EnemyPawn    = make_piece(Them, PAWN);
    constexpr Piece FriendlyKing = make_piece(Us,   KING);
    constexpr Piece EnemyKing    = make_piece(Them, KING);

    score += king_safety<Us>(lsb(bb(FriendlyKing)), bb(FriendlyPawn)) - king_safety<Them>(lsb(bb(EnemyKing)), bb(EnemyPawn));

    constexpr Bitboard Rank567 = Us == WHITE ? RANK_5BB | RANK_6BB | RANK_7BB : RANK_2BB | RANK_3BB | RANK_4BB;
    constexpr Bitboard Rank234 = Us == WHITE ? RANK_2BB | RANK_3BB | RANK_4BB : RANK_5BB | RANK_6BB | RANK_7BB;

    score += 4 * (popcount(bb(FriendlyPawn) & Rank567) -  popcount(bb(EnemyPawn) & Rank234));

    for (PieceType pt : { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING })
    {
        for (Bitboard b = bitboards[make_piece(Us, pt)]; b; clear_lsb(b))
            score += square_score<Us>(pt, lsb(b));

        for (Bitboard b = bitboards[make_piece(Them, pt)]; b; clear_lsb(b))
            score -= square_score<Them>(pt, lsb(b));
    }

    return score;
}

template<Color Us>
int endgame()
{
    int score = material_count<Us>();

    constexpr Color Them         = !Us;
    constexpr Piece FriendlyKing = make_piece(Us,   KING);
    constexpr Piece EnemyKing    = make_piece(Them, KING);

    score += end_king_squares[lsb(bb(FriendlyKing))];
    score -= end_king_squares[lsb(bb(EnemyKing   ))];

    constexpr Bitboard Rank2 = Us == WHITE ? RANK_2BB : RANK_7BB;
    constexpr Bitboard Rank3 = Us == WHITE ? RANK_3BB : RANK_6BB;
    constexpr Bitboard Rank4 = Us == WHITE ? RANK_4BB : RANK_5BB;
    constexpr Bitboard Rank5 = Us == WHITE ? RANK_5BB : RANK_4BB;
    constexpr Bitboard Rank6 = Us == WHITE ? RANK_6BB : RANK_3BB;
    constexpr Bitboard Rank7 = Us == WHITE ? RANK_7BB : RANK_2BB;

    Bitboard friendly_pawn = bitboard<make_piece(Us,   PAWN)>();
    Bitboard enemy_pawn    = bitboard<make_piece(Them, PAWN)>();

    score += 10 * popcount(friendly_pawn & Rank4);
    score -= 10 * popcount(enemy_pawn    & Rank5);
    score += 20 * popcount(friendly_pawn & Rank5);
    score -= 20 * popcount(enemy_pawn    & Rank4);
    score += 50 * popcount(friendly_pawn & Rank6);
    score -= 50 * popcount(enemy_pawn    & Rank3);
    score += 90 * popcount(friendly_pawn & Rank7);
    score -= 90 * popcount(enemy_pawn    & Rank2);

    return score;
}

template<Color Us>
int mopup()
{
    int score = material_count<Us>();

    constexpr Piece FriendlyKing = make_piece( Us, KING);
    constexpr Piece EnemyKing    = make_piece(!Us, KING);

    return score > 0 ? score + 10 * distance_from_center(lsb(bb(EnemyKing   ))) + 4 * (14 - square_distance(lsb(bb(FriendlyKing)), lsb(bb(EnemyKing))))
                     : score - 10 * distance_from_center(lsb(bb(FriendlyKing))) - 4 * (14 - square_distance(lsb(bb(FriendlyKing)), lsb(bb(EnemyKing))));
}

template<Color Perspective>int static_eval()
{
    return Position::midgame() ? midgame<Perspective>() : Position::endgame() ? endgame<Perspective>() : mopup<Perspective>();
}

#endif
