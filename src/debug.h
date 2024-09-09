
#ifndef DEBUG_H
#define DEBUG_H

#include <sstream>
#include <string>
#include <vector>

#include "types.h",

namespace Debug {

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

void go();
void gameinfo();
void perft(std::istringstream& args);

} // namespace Debug

#endif
