
#ifndef DEBUG_H
#define DEBUG_H

#include "types.h"
#include "ui.h"
#include <iostream>
#include <string>
#include <cstdint>
#include <fstream>

namespace Debug {

  inline std::string to_binary_string(int a) {
    std::string binary = "";
    int mask = 1;
    for (int i = 0; i < 31; i++) {
      if ((mask & a) >= 1)
        binary = "1" + binary;
      else
        binary = "0" + binary;
      mask <<= 1;
    }
    return binary;
  }

  inline void get_square(Square& s) {
    std::string str;
    std::cin >> str;
    for (Square sq = H1; sq <= A8; sq++) {
      if (UI::coords[sq] == str) {
        s = sq;
        return;
      }
    }
  }
  /*
    1. d4 d5 2. Nc3 Nf6 3. Bf4 e6 4. e3 Bb4 5. Qd3 Kf8 6. g3 Kg8
    7. Bg2 c5 8. dxc5 Nc6 9. Ne2 Bxc5 10. O-O-O Ng4 11. Rdf1 e5
    12. Bxe5 Ngxe5 13. Qd2 d4 14. exd4 Nxd4 15. Nxd4 Qxd4 16. f4 Nc4
    17. Qd3 Ne3 18. Qxd4 Bxd4 19. Rfg1 Nxg2 20. Rxg2 Bxc3 21. bxc3 Be6
    22. Rd2 Bxa2 23. Kb2 Be6 24. Rb1 h5 25. h4 b5 26. Rd4 f5 27. c4 Bxc4
    28. Kc1 a5 29. c3 a4 30. Ra1 Rh6 31. Ra3 Rg6 32. g4 Rxg4
    33. Rxc4 bxc4 34. Kc2 Rxh4 35. Kd2 Rd8+ 36. Ke3 Rh2 37. Kf3 Rh3+
    38. Kg2 Rh4 39. Kg3 Rg4+
  */

  // -#4 blunder (not reproducing)
  // 2Q5/p3kpp1/2p1p1p1/6r1/2PP3r/1P3q1P/P4PP1/3R1RK1 w - - 0 1

  // nd7 blunder
  // 3r1rk1/p1R2pp1/1n3n1p/4N3/3BP3/1B2P3/P5PP/6K1 b - - 0 1

  // rep
  // 2kb3r/2rR4/2pR1p2/3p4/pBpP2pp/P7/1P3PPP/5K2 w - - 0 1
  // position fen 2kb3r/2rR4/2pR1p2/3p4/pBpP2pp/P7/1P3PPP/5K2 w - - 0 1 moves d7c7 c8c7 d6e6 c7d7 e6d6 d7c7 d6e6 c7d7 e6d6


  // 1r1q1rk1/p5bp/B2pp1p1/2p1n3/3nP1b1/1PNP1p2/PBPQ1PPP/1R2NRK1 w - - 0 1
  // pawn blunder: 1rb3k1/3p1ppr/pp1b1q1p/3BpP1Q/1P2P3/2PP2R1/6PP/R1B4K b - - 1 1
  inline const char* perft0 = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 10";
  inline const char* perft1 = "r4rk1/pppbqppp/2np1n2/2b1p1B1/2B1P3/2NP1N2/PPP1QPPP/3R1RK1 w - - 0 1";
  inline const char* perft2 = "2rqkb1r/ppp2p2/2npb1p1/1N1Nn2p/2P1PP2/8/PP2B1PP/R1BQK2R b KQ - 0 11";
  inline const char* perft3 = "r1bq1rk1/ppp1nppp/4n3/3p3Q/3P4/1BP1B3/PP1N2PP/R4RK1 w - - 1 16";
  inline const char* pinmask = "8/2qqqqq1/2qQQQq1/2qQKQq1/2qQQQq1/2qqqqq1/8/k7 w - - 0 1";
  inline const char* rookmate = "3r4/3k4/8/8/3K4/8/8/8 w -- 0 1";
  inline const char* ttbug = "8/8/8/8/2r5/1k6/8/K7 b - - 0 1";
  inline const char* epbug = "rnbqk2r/ppp2ppp/8/3PP3/4n3/5Q2/PP3bPP/RNBK1BNR b KQkq - 0 1";
  inline const char* pawn_blunder = "r1bq1rk1/pppp1pp1/2nb1n1p/4p3/2B1P3/2NPBN2/PPP2PPP/R2Q1RK1 b - - 0 9";
  inline const char* pawn_blunder_ = "rnb1kb1r/ppp2ppp/5n2/4q3/N1BNp3/1P6/PBPP1PPP/R2QK2R b KQkq - 0 1";
  inline const char* pawn_blunder__ = "r3kbnr/p5pp/2pp1p2/q3p3/4P1bB/2N2N2/PPP2PPP/R2Q1RK1 b kq - 0 12";
  inline const char* knight_blunder = "r3r1k1/1ppq1ppp/p1nb4/5b2/3Pn3/P1NBPN1P/1PQ2PP1/R1B2RK1 b - - 0 1";
  inline const char* knight_blunder_ = "r4rk1/ppp2ppp/2b2q2/4n3/2N1pB2/4P3/PPP1QPPP/1K1R3R w - - 7 17";
  inline const char* mate_blunder = "3r2k1/R4ppp/5q2/P5R1/3P1n1K/5P2/2Q2P1P/6r1 b - - 8 27";
  inline const char* _knight_blunder = "r2qr1k1/ppp2ppp/2n2b2/8/3PpB2/1Q2P3/PP1N1PPP/R4RK1 b - - 3 14";

  inline int last_depth_searched;
  void boardstatus();
  void go();

}

#endif
