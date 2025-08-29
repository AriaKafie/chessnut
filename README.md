## Overview
Chessnut is a UCI-compatible chess engine written from scratch in C++.

## Compiling Chessnut
```
cd src
make && ./chessnut
```

On hardware that doesn't support BMI or BMI2 (for instructions like BLSR, PEXT, etc.) repeat those commands only after removing the following line in src/types.h:
```
#define BMI
```

## Interesting features
Chessnut has some interesting optimizations. For example, to update castling branchlessly, it uses a pext scheme (or magic bitboards if BMI isn't supported) to map every occupancy pattern on the relevant bitboard to a unique key that can be used to index a table of precomputed masks (to remove or preserve castling rights):
```
// position.h

template<Color JustMoved>
void update_castling_rights()
{
    constexpr Bitboard Mask = JustMoved == WHITE ? square_bb(A1, E1, H1, A8, H8) : square_bb(A8, E8, H8, A1, H1);
#ifdef BMI
    state_ptr->castling_rights &= CastleMasks[JustMoved][pext(bitboards[JustMoved], Mask)];
#else
    constexpr Bitboard Magic = JustMoved == WHITE ? 0x4860104020003061ull : 0x1080000400400c21ull;
    state_ptr->castling_rights &= CastleMasks[JustMoved][(bitboards[JustMoved] & Mask) * Magic >> 59];
#endif
}
```
It uses a similar scheme to evaluate king safety:
```
// bitboard.h

template<Color C>
int king_safety(Square ksq, Bitboard occ) {
#ifdef BMI
    return KingShieldScores[C][ksq][pext(occ, KingShield[C][ksq])];
#else
    occ &=  KingShield[C][ksq];
    occ *=  KingShieldMagics[C][ksq];
    occ >>= 58;
    return KingShieldScores[C][ksq][occ];
#endif
}
```
And to detect passed pawns in constant time:
```
// evaluation.h

template<Color Us>
Bitboard passers(Bitboard friendly_pawn, Bitboard opponent_pawn) {
    return friendly_pawn
        & Passers[Us][0][pext(opponent_pawn, Us == WHITE ? 0x03030303030000ull : 0xc0c0c0c0c000ull)]
        & Passers[Us][1][pext(opponent_pawn, Us == WHITE ? 0x0c0c0c0c0c0000ull : 0x303030303000ull)]
        & Passers[Us][2][pext(opponent_pawn, Us == WHITE ? 0x30303030300000ull : 0x0c0c0c0c0c00ull)]
        & Passers[Us][3][pext(opponent_pawn, Us == WHITE ? 0xc0c0c0c0c00000ull : 0x030303030300ull)];
}
```

## Commands

Set the board, optionally with moves in the UCI format:
```
position {startpos | fen <fen>} [moves m1 m2 ... mN]
```

Display the board:
```
d
```

Make moves:
```
moves m1 [m2 ... mN]
```

Start searching:
```
go [movetime <milliseconds> | wtime <wtime> winc <winc> btime <btime> binc <binc>]
```

Stop searching:
```
stop
```

Run perft (for sanity checks and benchmarking)
```
perft [depth]
```

Call Debug::go() (implemented in src/debug.cpp)
```
debug
```

