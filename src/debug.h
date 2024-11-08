
#ifndef DEBUG_H
#define DEBUG_H

#include <sstream>
#include <string>
#include <vector>

#include "types.h",

Move* get_moves(Move *list);

namespace Debug {

// 6rk/7p/Q2R1p2/1p2n3/4b3/1P4NP/P1P2PPK/2q5 b - - 0 1

inline std::vector<std::string> endgames =
{
    "8/p1p3pp/2k5/P1B5/1Pr3n1/5N2/r4PPP/3R2K1 w - - 2 26",
    "8/1RPk4/8/5p1p/5Pbr/4B3/5K2/8 b - - 2 42",
    "R7/P4pkp/4n1p1/3N4/8/7P/6PK/r7 b - - 2 40",
    "rn3rk1/pp3p1p/3p4/6pn/3P1p2/3B1R1P/PPP3P1/R1B3K1 w - - 0 15",
    "8/5pk1/1R2p1p1/6P1/1p2KP2/1r2P3/8/8 b - - 1 47",
    "7k/4P1np/1p6/p7/2P5/3R3P/4r1BK/8 b - - 2 48",
    "3r2k1/3Nn1pp/2b1pp2/1pNp4/3P3P/4P1P1/2n2PB1/5RK1 w - - 2 24",
    "8/5k2/4p3/1Q1p1ppp/3P4/q2KP1P1/5P1P/8 w - - 2 33",
    "6k1/6p1/2p5/p1p3P1/PrP1Pp2/1P2r3/1R3K2/1R6 w - - 3 37",
    "3n4/2r1k2p/2p1p3/N1N1Ppb1/2P5/1P5P/PB2K1P1/8 b - - 4 28",
    "r2r3k/6pp/p1b5/3p2p1/2P5/1B6/P4R1P/5RK1 w - - 0 29",
    "2b3k1/5r1p/6pP/p1Np2P1/1p1Ppr2/P7/1PPKR3/6R1 w - - 0 36",
    "8/p6p/4k3/8/1P2ppKP/2n5/1PP2P2/8 w - - 0 33",
    "4r1k1/p4p2/1ppr3p/6pP/2BB4/1P2P3/8/2K2R2 b - - 1 29",
    "8/1R6/4kp2/5p2/1P1p4/r6P/5PPK/8 w - - 0 34",
    "2br2k1/1p4p1/2p2p1p/2P1b3/1P1NB3/3RP2P/6P1/6K1 w - - 1 26",
    "r5r1/5p1k/p1ppbp2/1p2p2p/3bP1P1/1B1P1P1P/PPP1N3/1R2K2R b K - 1 19",
    "8/pkp5/2n2R2/1p2p1P1/3pPn2/PP1P1B2/1NP4r/1K6 b - - 2 43",
    "8/2p3pp/1ppkb3/6P1/1PP4P/P3K3/4B3/8 b - - 0 36",
    "r3r1k1/pp6/2pB1pp1/8/2bP4/2P2P2/6PP/2R1R1K1 w - - 2 27",
    "8/8/3Rk1p1/1pP4p/1P3PpK/6P1/8/r7 b - - 4 41",
    "3N2k1/8/1p4p1/pP3p1p/r7/P3R3/5K2/8 b - - 4 38",
    "5r1k/6p1/4P2p/8/4R3/1PB2PP1/r6P/7K b - - 0 36",
    "4r3/p1pN1pkp/3p4/5p2/1b6/PP2P1P1/2P1K2P/R7 b - - 0 24",
    "8/ppp3p1/3k1n1p/3p4/3P4/2P5/PP1N1KPP/8 w - - 2 26",
    "2r1brk1/pR6/4Pbp1/3p1pN1/7P/6B1/P7/5R1K w - - 1 27",
    "8/6k1/3p3p/6p1/2RpPr1P/5PKR/1r6/8 w - - 0 34",
    "2r2r2/ppp1Rpk1/3p2b1/3P4/PP1P3P/8/4BR2/6K1 b - a3 0 28",
    "8/4b3/3pk3/nrp1p1p1/4PpPp/3P1P1P/P2NKB2/R7 w - - 0 40",
    "7r/1pk1Rp2/2p2p2/p2p4/P1nP2Bp/2P4P/P4PP1/6K1 b - - 1 25",
    "r3rk2/pp1b2pp/2p5/1P2bp2/N7/2PB3P/P4PP1/R4RK1 w - - 1 21",
};

inline std::vector<std::string> fens =
{
    "r1bqr1k1/ppp2ppp/2pb1n2/4p3/P3P3/2NPBN2/1PP2PPP/R2Q1RK1 b - - 0 10",
    "rn1q1rk1/1bp1bpp1/1p2pn1p/p2p4/1PPP4/P4NP1/2QNPPBP/R1B2RK1 w - - 0 11",
    "r3k2r/pbppq1pp/1pn1p2n/1N3p2/2PPP3/P3BP2/1P4PP/R2QKB1R b KQkq - 4 10",
    "r1bqr1k1/pp1nbppp/2p2n2/3p4/3P4/2N2NP1/PPQBPPBP/R4RK1 w - - 0 11",
    "r1bq1rk1/1pp2ppp/pnpb4/4p3/4P3/2NP1N1P/PPP2PP1/R1BQ1RK1 b - - 4 10",
    "r2q1rk1/1ppb1ppp/4pn2/p1b5/2Pn1B2/2NP1NP1/PP3PBP/R2Q1RK1 w - - 4 11",
    "r1bqkb1r/p4ppp/p3pn2/2P5/N2p4/1Q3N2/PP1P1PPP/R1B2RK1 b kq - 1 10",
    "r1bqk2r/ppp2ppp/2n2n2/4p3/2P5/3PPNPP/PP4B1/RN1Q1RK1 b kq - 0 10",
    "r1bq1rk1/ppp2ppp/3p1n2/4p1B1/2B1P2n/2PP4/P1P2PPP/R2Q1RK1 w - - 0 11",
    "r1bqr1k1/pp1p1pp1/2pb1n1p/4p3/2BNP3/P1NP4/1PP2PPP/R1BQ1RK1 b - - 0 10",
    "r2qk2r/1p2bpp1/2p1pn1p/p2pNb2/Pn6/1P1PP1P1/1BPN1PBP/R2QK2R w KQkq - 0 11",
    "r1bqr1k1/pp3pp1/2n1pn1p/2bp4/8/2PBPNB1/PP1N1PPP/R2Q1RK1 w - - 0 11",
    "r1bqk2r/1p2bppp/p1n1pn2/8/P2P4/2NB1N2/1P3PPP/R1BQ1RK1 b kq - 0 10",
    "r1bq1rk1/1pp1ppbp/p1n3p1/8/P2Pp3/3BBN1P/1PP2PP1/R2QK2R w KQ - 0 11",
    "rnb1k2r/1p2bppp/p3pn2/8/1PB5/P3PN2/3N1PPP/R1BK3R b kq - 0 10",
    "r2qk2r/ppp2pp1/2np3p/2b5/2P1b2P/4PNP1/PP3PB1/R1BQK2R w KQkq - 0 11",
    "rnbq1rk1/pp3pp1/4pn1p/3p4/2PN3B/2Q1P3/PP3PPP/R3KB1R b KQ - 0 10",
    "r1bqr1k1/p2n1ppp/1pp1pn2/3p4/1bPP4/1PNQ1NP1/PB2PPBP/R4RK1 b - - 2 10",
    "r1bq1rk1/1pp2ppp/1bnp1n2/4p3/1pB1P3/P1N2N1P/2PP1PP1/1RBQ1RK1 w - - 0 11",
    "r1bq1rk1/pp1n1ppp/2pbpn2/3p4/2PP3P/1P2PN2/PBRN1PP1/3QKB1R b K - 6 10",
    "r1b1kb1r/ppp2pp1/2n1qn1p/8/8/2NQ2P1/PPN1PPBP/R1B1K2R b KQkq - 0 10",
    "rn1q1rk1/pbp1bpp1/1p2pn1p/3p4/2PP4/PP3NP1/1B1NPPBP/R2Q1RK1 b - - 2 10",
    "r3k2r/p1p1qppp/p2b1n2/3p1b2/3P4/1P2P2P/P2N1PP1/RNBQ1RK1 w kq - 3 11",
    "2kr1bnr/pppq1ppp/2n5/4pbN1/8/2NPB1PP/PPP2PB1/R2QK2R b KQ - 2 10",
    "r1bq1rk1/p3bpp1/1p2pn1p/2ppB3/P7/3P2P1/1PP1PPBP/RN1QR1K1 w - - 0 11",
    "r2q1rk1/1pp1ppbp/2n3p1/3n4/p5b1/P1NP1NP1/1PQ1PPBP/R1B2RK1 w - - 2 11",
    "r2q1rk1/1pp2pp1/p1npbn1p/2bNp3/P1B1P3/3P1N2/1PP1QPPP/R1B2RK1 w - - 2 11",
    "r2qkb1r/1p1n1pp1/2p1pn1p/p1PpNb2/3P4/1Q4PP/PP1NPPB1/R1B1K2R w KQkq - 1 11",
    "r2q1rk1/pbpnbppp/1p2pn2/8/P1pP4/2B2NP1/1P1NPPBP/R2Q1RK1 w - - 6 11",
    "r1bq1rk1/p1pn1pp1/1p2p2p/3n4/3P4/P1B1PN2/1PQ2PPP/R3KB1R w KQ - 0 11",
    "r1bq1rk1/1p2npbp/2np2p1/p1p1p3/Q1P5/P1NPP1P1/1P2NPBP/R1B2RK1 b - - 3 10"
};

std::string pv();
void go();
void perft(std::istringstream& args);

} // namespace Debug

#endif
