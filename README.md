## Overview
Chessnut is a UCI-compatible chess engine written from scratch in c++.

## Compiling Chessnut
```
cd src
make && ./chessnut
```

On hardware that doesn't support BMI or BMI2 (for instructions like BLSR, PEXT, etc.) repeat these commands only after removing the following line in src/types.h:
```
#define BMI
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
