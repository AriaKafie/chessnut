
#include "bitboard.h"

#include <algorithm>

Bitboard attacks_bb(PieceType pt, Square sq, Bitboard occupied)
{
    Bitboard  attacks   = 0;
    Direction rook[4]   = { NORTH, EAST, SOUTH, WEST };
    Direction bishop[4] = { NORTH_EAST, SOUTH_EAST, SOUTH_WEST, NORTH_WEST };

    for (Direction d : pt == ROOK ? rook : bishop)
    {
        Square s = sq;
        
        do
            attacks |= safe_step(s, d);
        while (safe_step(s, d) && !(square_bb(s += d) & occupied));
    }
    
    return attacks;
}

void Bitboards::init()
{
    for (Square s1 = H1; s1 <= A8; s1++)
    {
        FileBB[s1] = FILE_H << s1 % 8;

        for (Square s2 = H1; s2 <= A8; s2++)
            SquareDistance[s1][s2] = std::max(file_distance(s1, s2), rank_distance(s1, s2));
    }

    for (Square s1 = H1; s1 <= A8; s1++)
    {
        MainDiag[s1] = attacks_bb(BISHOP, s1, 0) & (mask(s1, NORTH_WEST) | mask(s1, SOUTH_EAST)) | square_bb(s1);
        AntiDiag[s1] = attacks_bb(BISHOP, s1, 0) & (mask(s1, NORTH_EAST) | mask(s1, SOUTH_WEST)) | square_bb(s1);

        for (Square s2 = H1; s2 <= A8; s2++)
            if (PieceType pt; attacks_bb(pt=BISHOP, s1, 0) & square_bb(s2) || attacks_bb(pt=ROOK, s1, 0) & square_bb(s2))
            {
                CheckRay [s1][s2] = attacks_bb(pt, s1, square_bb(s2)) & attacks_bb(pt, s2, square_bb(s1)) | square_bb(s2);
                AlignMask[s1][s2] = attacks_bb(pt, s1, 0)             & attacks_bb(pt, s2, 0)             | square_bb(s1, s2);
            }

        for (Direction d : { NORTH, NORTH_EAST, EAST, SOUTH_EAST, SOUTH, SOUTH_WEST, WEST, NORTH_WEST })
            KingAttacks[s1] |= safe_step(s1, d);

        for (Direction d : { NORTH+NORTH_EAST, NORTH_EAST+EAST, SOUTH_EAST+EAST, SOUTH+SOUTH_EAST,
                             SOUTH+SOUTH_WEST, SOUTH_WEST+WEST, NORTH_WEST+WEST, NORTH+NORTH_WEST })
            KnightAttacks[s1] |= safe_step(s1, d);

        DoubleCheck[s1] = KingAttacks[s1] | KnightAttacks[s1];

        PawnAttacks[WHITE][s1] = pawn_attacks<WHITE>(square_bb(s1));
        PawnAttacks[BLACK][s1] = pawn_attacks<BLACK>(square_bb(s1));
    }
    
    uint8_t clearK = ~8;
    uint8_t clearQ = ~4;
    uint8_t cleark = ~2;
    uint8_t clearq = ~1;

    for (int i = 0; i < 1 << 5; i++)
    {
        Bitboard w_occ = generate_occupancy(square_bb(A1, E1, H1, A8, H8), i);
        Bitboard b_occ = generate_occupancy(square_bb(A8, E8, H8, A1, H1), i);

        uint8_t w_rights = 0xf;
        uint8_t b_rights = 0xf;

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
}