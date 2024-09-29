
#include "bitboard.h"

#include <algorithm>

int score_kingshield(Square ksq, Bitboard occ, Color c);

void init_magics();

Bitboard sliding_attacks(PieceType pt, Square sq, Bitboard occupied) {

    Direction rook_directions[4] = { NORTH, EAST, SOUTH, WEST };
    Direction bishop_directions[4] = { NORTH_EAST, SOUTH_EAST, SOUTH_WEST, NORTH_WEST };

    Bitboard atk = 0;

    for (Direction d : (pt == ROOK) ? rook_directions : bishop_directions)
        for (Square s = sq; safe_step(s, d) && !(square_bb(s) & occupied); atk |= square_bb(s += d));
    
    return atk;
}

void Bitboards::init() {

    for (Square s1 = H1; s1 <= A8; s1++)
    {
        FileBB[s1] = FILE_H << (s1 % 8);

        for (Square s2 = H1; s2 <= A8; s2++)
            SquareDistance[s1][s2] = std::max(file_distance(s1, s2), rank_distance(s1, s2));
    }

    init_magics();
    
    Bitboard* table_ptr = xray_table;

    for (PieceType pt : { BISHOP, ROOK })
    {
        Bitboard* masks = pt == BISHOP ? bishop_masks : rook_masks;

        for (Square s = H1; s <= A8; s++)
            for (Bitboard occupied = 0, i = 0; i < 1 << popcount(masks[s]); occupied = generate_occupancy(masks[s], ++i))
                *table_ptr++ = sliding_attacks(pt, s, occupied ^ (sliding_attacks(pt, s, occupied) & occupied));
    }

#define mdiag(s) (square_bb(s) | bishop_attacks(s, 0) & (mask(s, NORTH_WEST) | mask(s, SOUTH_EAST)))
#define adiag(s) (square_bb(s) | bishop_attacks(s, 0) & (mask(s, NORTH_EAST) | mask(s, SOUTH_WEST)))

#define md(a, b) (rank_distance(a, b) + file_distance(a, b))

    for (Square s1 = H1; s1 <= A8; s1++)
    {
        MainDiag[s1] = mdiag(s1);
        AntiDiag[s1] = adiag(s1);
            
        CenterDistance[s1] = std::min({ md(s1, E4), md(s1, E5), md(s1, D4), md(s1, D5) });

        for (Square s2 = H1; s2 <= A8; s2++)
            AlignMask[s1][s2] = mdiag(s1) & mdiag(s2) | adiag(s1) & adiag(s2) | rank_bb(s1) & rank_bb(s2) | file_bb(s1) & file_bb(s2);

        for (Square ksq = s1, checker = H1; checker <= A8; checker++)
        {
            for (Direction d : { NORTH, NORTH_EAST, EAST, SOUTH_EAST, SOUTH, SOUTH_WEST, WEST, NORTH_WEST })
            {
                Bitboard ray = 0;

                for (Square s = ksq; safe_step(s, d) && !(square_bb(s) & square_bb(checker)); ray |= square_bb(s += d));

                if (ray & square_bb(checker))
                    CheckRay[ksq][checker] = ray;
            }
        }

        for (Direction d : { NORTH, NORTH_EAST, EAST, SOUTH_EAST, SOUTH, SOUTH_WEST, WEST, NORTH_WEST })
            KingAttacks[s1] |= safe_step(s1, d);

        for (Direction d : { NORTH+NORTH_EAST, NORTH_EAST+EAST, SOUTH_EAST+EAST, SOUTH+SOUTH_EAST,
                             SOUTH+SOUTH_WEST, SOUTH_WEST+WEST, NORTH_WEST+WEST, NORTH+NORTH_WEST })
            KnightAttacks[s1] |= safe_step(s1, d);

        Square sq = 8 * (s1 / 8) + 1;

        white_kingshield[s1] =
            ((rank_bb(sq + NORTH) | rank_bb(sq + NORTH+NORTH)) & ~(mask(sq + WEST, WEST))) << std::min(5, std::max(0, (s1 % 8) - 1));

        black_kingshield[s1] =
            ((rank_bb(sq + SOUTH) | rank_bb(sq + SOUTH+SOUTH)) & ~(mask(sq + WEST, WEST))) << std::min(5, std::max(0, (s1 % 8) - 1));

        DoubleCheck[s1] = KingAttacks[s1] | KnightAttacks[s1];

        PawnAttacks[WHITE][s1] = pawn_attacks<WHITE>(square_bb(s1));
        PawnAttacks[BLACK][s1] = pawn_attacks<BLACK>(square_bb(s1));
    }

#undef md
#undef adiag
#undef mdiag

    uint8_t clearK = 0b0111;
    uint8_t clearQ = 0b1011;
    uint8_t cleark = 0b1101;
    uint8_t clearq = 0b1110;

    for (int i = 0; i < 1 << 5; i++)
    {
        Bitboard w_occ = generate_occupancy(square_bb(A1, E1, H1, A8, H8), i);
        Bitboard b_occ = generate_occupancy(square_bb(A8, E8, H8, A1, H1), i);

        uint8_t w_rights = 0b1111;
        uint8_t b_rights = 0b1111;

        if ((w_occ & square_bb(A1)) == 0) w_rights &= clearQ;
        if ((w_occ & square_bb(E1)) == 0) w_rights &= clearK & clearQ;
        if ((w_occ & square_bb(H1)) == 0) w_rights &= clearK;
        if ((w_occ & square_bb(A8)) != 0) w_rights &= clearq;
        if ((w_occ & square_bb(H8)) != 0) w_rights &= cleark;

        if ((b_occ & square_bb(A8)) == 0) b_rights &= clearq;
        if ((b_occ & square_bb(E8)) == 0) b_rights &= cleark & clearq;
        if ((b_occ & square_bb(H8)) == 0) b_rights &= cleark;
        if ((b_occ & square_bb(A1)) != 0) b_rights &= clearQ;
        if ((b_occ & square_bb(H1)) != 0) b_rights &= clearK;

        castle_masks[WHITE][pext(w_occ, square_bb(A1, E1, H1, A8, H8))] = w_rights;
        castle_masks[BLACK][pext(b_occ, square_bb(A8, E8, H8, A1, H1))] = b_rights;
    }

    for (Square sq = H1; sq <= A8; sq++)
    {
        for (Bitboard occupied = 0, i = 0; i < 1 << popcount(white_kingshield[sq]); occupied = generate_occupancy(white_kingshield[sq], ++i))
            white_kingshield_scores[sq][pext(occupied, white_kingshield[sq])] = score_kingshield(sq, occupied, WHITE);

        for (Bitboard occupied = 0, i = 0; i < 1 << popcount(black_kingshield[sq]); occupied = generate_occupancy(black_kingshield[sq], ++i))
            black_kingshield_scores[sq][pext(occupied, black_kingshield[sq])] = score_kingshield(sq, occupied, BLACK);
    }
}

void init_magics() {

    Bitboard* table_ptr = pext_table;

    for (PieceType pt : { BISHOP, ROOK })
    {
        int*      base = pt == BISHOP ? bishop_base  : rook_base;
        Bitboard* mask = pt == BISHOP ? bishop_masks : rook_masks;

        for (Square s = H1; s <= A8; s++)
        {
            base[s] = table_ptr - pext_table;
            mask[s] = sliding_attacks(pt, s, 0) & ~((FILE_A | FILE_H) & ~file_bb(s) | (RANK_1 | RANK_8) & ~rank_bb(s));

            for (int i = 0; i < 1 << popcount(mask[s]); i++)
                *table_ptr++ = sliding_attacks(pt, s, generate_occupancy(mask[s], i));
        }
    }
}

int score_kingshield(Square ksq, Bitboard occ, Color c) {

    constexpr int MAX_SCORE =  50;
    constexpr int MIN_SCORE = -50;

    Bitboard home_rank = (c == WHITE) ? RANK_1 : RANK_8;

    if (!(square_bb(ksq) & home_rank) || (popcount(occ) < 2))
        return MIN_SCORE;

    if (square_bb(ksq) & (FILE_E | FILE_D))
        return 10;

    constexpr int pawn_weights[8][6] = 
    { // [H1..A1][LSB..MSB]
        {10, 20, 15, 5, 10, 5},{5,    25, 15, 0, 0,    5},
        {20, 15, 10, 0, 0,    0},{0,    0,    0,    0, 0,    0},
        {0,    0,    0,    0, 0,    0},{10, 15, 20, 0, 0,    0},
        {15, 25, 5,    5, 0,    0},{15, 20, 10, 5, 10, 5}
    };

    constexpr int file_weights[8][3] =
    { // [H1..A1][right, middle, left]
        {40, 50, 45},{40, 50, 45},
        {50, 45, 40},{0,    0,     0},
        {0,    0,     0},{40, 45, 50},
        {45, 50, 40},{45, 50, 40}
    };

    Bitboard shield_mask = (c == WHITE) ? white_kingshield[ksq] : black_kingshield[ksq];

    Bitboard file_right = file_bb(lsb(shield_mask));
    Bitboard file_mid     = file_right << 1;
    Bitboard file_left    = file_right << 2;

    int score = 0;
    if (c == BLACK) ksq -= 56;
    if ((occ & file_right) == 0) score -= file_weights[ksq][0];
    if ((occ & file_mid)     == 0) score -= file_weights[ksq][1];
    if ((occ & file_left)    == 0) score -= file_weights[ksq][2];

    for (int i = 0; shield_mask; i++) {
        int index = (c == WHITE) ? i : ((i < 3) ? (i + 3) : (i - 3));
        Square sq = lsb(shield_mask);
        if (occ & square_bb(sq))
            score += pawn_weights[ksq][index];
        clear_lsb(shield_mask);
    }
    
    return std::max(MIN_SCORE, std::min(MAX_SCORE, score));
}

