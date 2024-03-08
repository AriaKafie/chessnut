
#ifndef BENCH_H
#define BENCH_H

#include <cstdint>
#include <vector>
#include <string>

inline std::vector<std::string> fens = {
  "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 10",
  "r4rk1/pppbqppp/2np1n2/2b1p1B1/2B1P3/2NP1N2/PPP1QPPP/3R1RK1 w - - 0 1",
  "2rqkb1r/ppp2p2/2npb1p1/1N1Nn2p/2P1PP2/8/PP2B1PP/R1BQK2R b KQ - 0 11",
  "r1bq1rk1/ppp1nppp/4n3/3p3Q/3P4/1BP1B3/PP1N2PP/R4RK1 w - - 1 16",
  "r2qk2r/pp2bppp/3pbn2/2p3B1/4P3/2NB4/PPP2PPP/R2QK2R b KQkq - 7 10",
  "3Q1rk1/7p/1p4p1/4pp2/1pP5/1P4P1/P4P1P/1R4K1 b - - 0 23",
  "rn1q1rk1/1b2bppp/nPp1p3/p1ppP3/3P4/2NB4/P4PPP/R1BQNRK1 w - - 1 14",
  "3rr3/pp1nnpkp/1q4p1/3pPpN1/3p4/2P1Q3/PP2NPPP/R4RK1 w - - 0 17",
  "r1bqkb1r/p5pp/2pppn2/8/4PP2/2N5/PP4PP/R1BQKB1R b KQkq - 1 11",
  "r1bq1rk1/3nbppp/p1p2n2/1p2p3/4P3/P1N2N2/BPP2PPP/R1BQR1K1 w - - 0 11",
  "1r2k2r/1p1q1p2/p1p4p/3p2p1/3Pb3/1NP1P1P1/P4QPP/2R2RK1 b k - 1 22",
  "3rkb1r/6p1/2p3Q1/8/4p3/2n1P1Pp/P1qB1P1P/3RKR2 b k - 5 27",
  "rnb1kb1r/ppp1pppp/8/3q4/8/8/PPPP1PPP/R1BQKBNR w KQkq - 0 5",
  "rnb1k2r/ppp1bppp/5n2/8/2BpN3/5N2/PPP2PPP/R1B1K2R w KQkq - 3 8",
  "r1bqkbnr/pp1ppppp/2n5/1Bp5/4P3/2P5/PP1P1PPP/RNBQK1NR b KQkq - 2 3",
  "r2qkb1r/pp1b1ppp/2n1pn2/2p1P3/2B5/5N2/PBPP1PPP/RN1Q1RK1 b kq - 0 8",
  "r2qkbnr/pp1b2pp/2n1pp2/3p4/1P1P4/P3P3/3B1PPP/RN1QKBNR b KQkq - 0 8",


  "2rq1rk1/4ppbp/n2p2p1/p2nP1B1/1p1P4/5NN1/PP3PPP/R2QR1K1 w - - 1 16",
  "r4rk1/pp1n1ppp/1qpb1n2/3p3b/3P4/2N1PN1P/PPQ1BPP1/R1B2RK1 w - - 7 13",
  "rnbqk1nr/ppp1ppbp/6p1/3p4/P1PP4/4P3/1P3PPP/RNBQKBNR b KQkq c3 0 4",
  "r1b1k1nr/ppp2ppp/1b3q2/1B1Pp3/4P3/PQN2P2/6PP/R1B2K1R b kq - 2 15",
  "rn1qkb1r/ppp2ppp/4pn2/3p4/3P2b1/3BPN2/PPP2PPP/RNBQK2R w KQkq - 0 5",
  "r2qkb1r/pp1n1ppp/4pn2/3p1b2/3P4/2N1PN2/PP2BPPP/R1BQ1RK1 b kq - 1 8",
  "1r3k1r/ppp3pp/2npbq2/1Q6/3pP3/5N2/PPP2PPP/R1B1K2R w KQ - 8 14",
  "r1bq1rk1/ppp1bppp/2np3n/4P3/5P2/2NB1N1P/PPP3P1/R1BQ1RK1 b - - 1 11"
};

namespace Bench {
  void count_nodes(int depth);
  inline int node_count;
  inline int q_node_count;
}

#endif
