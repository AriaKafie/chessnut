
#ifndef DEBUG_H
#define DEBUG_H

#include <string>
#include <cstdint>
#include <fstream>

namespace Chess {

namespace Debug {

	static constexpr const char* perft0 = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq 0 1";
	static constexpr const char* perft1 = "r4rk1/pppbqppp/2np1n2/2b1p1B1/2B1P3/2NP1N2/PPP1QPPP/3R1RK1 w - - 0 1";
	static constexpr const char* perft2 = "2rqkb1r/ppp2p2/2npb1p1/1N1Nn2p/2P1PP2/8/PP2B1PP/R1BQK2R b KQ - 0 11";
	static constexpr const char* perft3 = "r1bq1rk1/ppp1nppp/4n3/3p3Q/3P4/1BP1B3/PP1N2PP/R4RK1 w - - 1 16";
	static constexpr const char* pinmask = "8/2qqqqq1/2qQQQq1/2qQKQq1/2qQQQq1/2qqqqq1/8/k7 w - - 0 1";
	static constexpr const char* rookmate = "3r4/3k4/8/8/3K4/8/8/8 w -- 0 1";
	static constexpr const char* ttbug = "8/8/8/8/2r5/1k6/8/K7 b - - 0 1";
	static constexpr const char* epbug = "rnbqk2r/ppp2ppp/8/3PP3/4n3/5Q2/PP3bPP/RNBK1BNR b KQkq - 0 1";
	static constexpr const char* pawn_blunder = "r1bq1rk1/pppp1pp1/2nb1n1p/4p3/2B1P3/2NPBN2/PPP2PPP/R2Q1RK1 b - - 0 9";
	static constexpr const char* pawn_blunder_ = "rnb1kb1r/ppp2ppp/5n2/4q3/N1BNp3/1P6/PBPP1PPP/R2QK2R b KQkq - 0 1";
	static constexpr const char* pawn_blunder__ = "r3kbnr/p5pp/2pp1p2/q3p3/4P1bB/2N2N2/PPP2PPP/R2Q1RK1 b kq - 0 12";
	static constexpr const char* knight_blunder = "r3r1k1/1ppq1ppp/p1nb4/5b2/3Pn3/P1NBPN1P/1PQ2PP1/R1B2RK1 b - - 0 1";
	static constexpr const char* knight_blunder_ = "r4rk1/ppp2ppp/2b2q2/4n3/2N1pB2/4P3/PPP1QPPP/1K1R3R w - - 7 17";
	static constexpr const char* mate_blunder = "3r2k1/R4ppp/5q2/P5R1/3P1n1K/5P2/2Q2P1P/6r1 b - - 8 27";
	static constexpr const char* _knight_blunder = "r2qr1k1/ppp2ppp/2n2b2/8/3PpB2/1Q2P3/PP1N1PPP/R4RK1 b - - 3 14";

	inline int last_depth_searched;
	void boardstatus();
	void go();
	void TT_status();
	void save_table_image();
	void clear_txt();
	void load_custom_tt(std::string path);
	void thing(uint64_t mask);
	void print_bitboard_to_file(std::ofstream& o, uint64_t bb);
	void test();

}

}

#endif
