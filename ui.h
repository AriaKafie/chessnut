
#ifndef UI_H
#define UI_H
#include <string>

namespace Chess {

namespace UI {

	inline std::string coords[64] = {
		"h1", "g1", "f1", "e1", "d1", "c1", "b1", "a1",
		"h2", "g2", "f2", "e2", "d2", "c2", "b2", "a2",
		"h3", "g3", "f3", "e3", "d3", "c3", "b3", "a3",
		"h4", "g4", "f4", "e4", "d4", "c4", "b4", "a4",
		"h5", "g5", "f5", "e5", "d5", "c5", "b5", "a5",
		"h6", "g6", "f6", "e6", "d6", "c6", "b6", "a6",
		"h7", "g7", "f7", "e7", "d7", "c7", "b7", "a7",
		"h8", "g8", "f8", "e8", "d8", "c8", "b8", "a8",
	};
	inline std::string type_to_char = "PNBRQKpnbrqk ";
	void print_board();
	void move_prompt();
	void clear_line();
	std::string move_to_string(int move);
	int movestring_to_int(std::string move);

}

}

#endif
