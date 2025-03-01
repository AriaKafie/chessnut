
#include "bitboard.h"

#include <algorithm>
#ifndef PEXT
    #include <cstring>
    #include <random>
#endif

void init_magics();

Bitboard generate_occupancy(Bitboard mask, int permutation)
{
    Bitboard occupied = 0;

    for (;mask; clear_lsb(mask), permutation >>= 1)
        if (permutation & 1) occupied |= mask & (mask ^ 0xffffffffffffffff) + 1;

    return occupied;
}

Bitboard attacks_bb(PieceType pt, Square sq, Bitboard occupied)
{
    Bitboard  attacks              = 0;
    Direction rook_directions[4]   = { NORTH, EAST, SOUTH, WEST };
    Direction bishop_directions[4] = { NORTH_EAST, SOUTH_EAST, SOUTH_WEST, NORTH_WEST };

    for (Direction d : pt == BISHOP ? bishop_directions : rook_directions)
    {
        Square s = sq;

        do
            attacks |= safe_step(s, d);
        while (safe_step(s, d) && !(square_bb(s += d) & occupied));
    }
    
    return attacks;
}

/*#ifndef PEXT
uint64_t generate_magic(uint64_t mask)
{
    int permutations = 1 << popcount(mask);

    uint64_t occupied[4096], magic;
    bool visited[4096], failed;

    for (int p = 0; p < permutations; p++)
        occupied[p] = generate_occupancy(mask, p);

    std::mt19937_64 rng(0);

    do
    {
        magic = rng() & rng() & rng();
        memset(visited, false, permutations);

        for (int p = 0, key = 0; p < permutations; key = occupied[++p] * magic >> 64 - popcount(mask))
        {
            if (failed = visited[key])
                break;

            visited[key] = true;
        }
        
    } while (failed);

    return magic;
}
#endif*/

void Bitboards::init()
{
    for (Square s1 = H1; s1 <= A8; s1++)
    {
        FileBB[s1] = FILE_H << s1 % 8;

        for (Square s2 = H1; s2 <= A8; s2++)
            SquareDistance[s1][s2] = std::max(file_distance(s1, s2), rank_distance(s1, s2));
    }

    init_magics();

    for (Square s1 = H1; s1 <= A8; s1++)
    {
        MainDiag[s1] = bishop_attacks(s1, 0) & (mask(s1, NORTH_WEST) | mask(s1, SOUTH_EAST)) | square_bb(s1);
        AntiDiag[s1] = bishop_attacks(s1, 0) & (mask(s1, NORTH_EAST) | mask(s1, SOUTH_WEST)) | square_bb(s1);

        CenterDistance[s1] = std::min({ rank_distance(s1, E4) + file_distance(s1, E4),
                                        rank_distance(s1, E5) + file_distance(s1, E5),
                                        rank_distance(s1, D4) + file_distance(s1, D4),
                                        rank_distance(s1, D5) + file_distance(s1, D5) });

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

        Square sq = s1 & ~7 | 1;

        KingShield[WHITE][s1] =
            ((rank_bb(sq + NORTH) | rank_bb(sq + NORTH+NORTH)) & ~(mask(sq + WEST, WEST))) << std::clamp(s1 % 8 - 1, 0, 5);

        KingShield[BLACK][s1] =
            ((rank_bb(sq + SOUTH) | rank_bb(sq + SOUTH+SOUTH)) & ~(mask(sq + WEST, WEST))) << std::clamp(s1 % 8 - 1, 0, 5);

        DoubleCheck[s1] = KingAttacks[s1] | KnightAttacks[s1];

        PawnAttacks[WHITE][s1] = pawn_attacks<WHITE>(square_bb(s1));
        PawnAttacks[BLACK][s1] = pawn_attacks<BLACK>(square_bb(s1));
    }

    for (int i = 0; i < 1 << 5; i++)
    {
        const uint8_t clearK = ~8;
        const uint8_t clearQ = ~4;
        const uint8_t cleark = ~2;
        const uint8_t clearq = ~1;

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

        CastleMasks[WHITE][pext(w_occ, square_bb(A1, E1, H1, A8, H8))] = w_rights;
        CastleMasks[BLACK][pext(b_occ, square_bb(A8, E8, H8, A1, H1))] = b_rights;
    }

    for (Color c : { WHITE, BLACK })
        for (Square ksq = H1; ksq <= A8; ksq++)
            for (int i = 0; i < 1 << popcount(KingShield[c][ksq]); i++)
            {
                Bitboard king_shield = generate_occupancy(KingShield[c][ksq], i);

                const int MIN_SCORE = -45, MAX_SCORE = 45;

                if (!(square_bb(ksq) & (c == WHITE ? RANK_1 : RANK_8)) || (popcount(king_shield) < 2))
                {
                    KingShieldScores[c][ksq][i] = MIN_SCORE;
                    continue;
                }

                if (square_bb(ksq) & (FILE_E | FILE_D))
                {
                    KingShieldScores[c][ksq][i] = 10;
                    continue;
                }

                const int pawn_weights[8][6] =
                { // [H...A][LSB...MSB]
                    { 10, 20, 15, 5, 10, 5 }, { 5,  25, 15, 0, 0,  5 },
                    { 20, 15, 10, 0, 0,  0 }, { 0,  0,  0,  0, 0,  0 },
                    { 0,  0,  0,  0, 0,  0 }, { 10, 15, 20, 0, 0,  0 },
                    { 15, 25, 5,  5, 0,  0 }, { 15, 20, 10, 5, 10, 5 }
                };

                const int file_weights[8][3] =
                { // [H...A][right, middle, left]
                    { 40, 50, 45 }, {40, 50, 45 },
                    { 50, 45, 40 }, {0,  0,  0  },
                    { 0,  0,  0  }, {40, 45, 50 },
                    { 45, 50, 40 }, {45, 50, 40 }
                };

                Bitboard shield_mask  = KingShield[c][ksq];
                Bitboard file_right   = file_bb(lsb(shield_mask));
                Bitboard file_mid     = file_right << 1;
                Bitboard file_left    = file_right << 2;
                int      score        = 0;
                int      king_file    = ksq & 7;

                if ((king_shield & file_right) == 0) score -= file_weights[king_file][0];
                if ((king_shield & file_mid)   == 0) score -= file_weights[king_file][1];
                if ((king_shield & file_left)  == 0) score -= file_weights[king_file][2];

                for (int i = 0; shield_mask; clear_lsb(shield_mask), i++)
                {
                    int index = (c == WHITE) ? i : ((i < 3) ? (i + 3) : (i - 3));

                    if (king_shield & square_bb(lsb(shield_mask)))
                        score += pawn_weights[king_file][index];
                }

                KingShieldScores[c][ksq][i] = std::clamp(score, MIN_SCORE, MAX_SCORE);
            }
}

void init_magics()
{
    #ifdef PEXT
        Bitboard *pext = pext_table, *xray = xray_table;

        for (PieceType pt : { BISHOP, ROOK })
        {
            int      *base = pt == BISHOP ? bishop_base  : rook_base;
            Bitboard *mask = pt == BISHOP ? bishop_masks : rook_masks;

            for (Square s = H1; s <= A8; s++)
            {
                base[s] = pext - pext_table;
                mask[s] = attacks_bb(pt, s, 0) & ~((FILE_A | FILE_H) & ~file_bb(s) | (RANK_1 | RANK_8) & ~rank_bb(s));

                for (Bitboard occupied = 0, i = 0; i < 1 << popcount(mask[s]); occupied = generate_occupancy(mask[s], ++i))
                {
                    *pext++ = attacks_bb(pt, s, occupied);
                    *xray++ = attacks_bb(pt, s, occupied ^ attacks_bb(pt, s, occupied) & occupied);
                }
            }
        }
    #else
        Bitboard magic, occupied[4096], attacks[4096], *base = pext_table;

        for (PieceType pt : { BISHOP, ROOK })
            for (Square s = H1; s <= A8; s++)
            {
                Bitboard mask = attacks_bb(pt, s, 0) & ~((FILE_A | FILE_H) & ~file_bb(s) | (RANK_1 | RANK_8) & ~rank_bb(s));

                Magic& m            = pt == BISHOP ? bishop_magics[s] : rook_magics[s];
                int    permutations = 1 << popcount(mask);
                int    shift        = 64 - popcount(mask);
                bool   failed       = false;

                m.ptr   = base;
                m.mask  = mask;
                m.shift = shift;

                for (int p = 0; p < permutations; p++)
                {
                    occupied[p] = generate_occupancy(mask, p);
                    attacks [p] = attacks_bb(pt, s, occupied[p]);
                }

                std::mt19937_64 rng(0);

                do
                {
                    magic = rng() & rng() & rng();
                    memset(base, 0, sizeof(Bitboard) * permutations);

                    for (int p = 0, key = 0; p < permutations; key = occupied[++p] * magic >> shift)
                    {
                        if (failed = base[key] && (base[key] != attacks[p]))
                            break;

                        base[key] = attacks[p];
                    }

                } while (failed);

                m.magic = magic;
                base += permutations;
            }
    #endif
}

std::string to_string(Bitboard b)
{
    std::string l = "+---+---+---+---+---+---+---+---+\n", s = l;

    for (Bitboard bit = square_bb(A8); bit; bit >>= 1)
    {
        s += bit & b ? "| @ " : "|   ";

        if (bit & FILE_H)
            s += "|\n" + l;
    }

    return s + "\n";
}
