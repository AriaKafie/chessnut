
#ifndef AI_H
#define AI_H

#include <cstdint>
#include <string>

#define WHITE_TRUE true
#define WHITE_FALSE false

namespace Chess {

namespace Search {

	std::string probe_white(int thinktime);
	std::string probe_black(int thinktime);

	int search(int alpha, int beta, bool maximizing, int depth, int ply_from_root);
	int quiescence_search(int alpha, int beta, bool maximizing);
	inline bool in_search;
	inline bool search_cancelled;

	inline bool is_alpha_matescore(int score) {
		return score > 90000;
	}
	inline bool is_beta_matescore(int score) {
		return score < -90000;
	}

	inline constexpr int depth_reduction[90] = {
		0,0,0,0,0,0,0,0,0,0,
		1,1,1,1,1,1,1,1,1,1,
		2,2,2,2,2,2,2,2,2,2,
		3,3,3,3,3,3,3,3,3,3,
		4,4,4,4,4,4,4,4,4,4,
		5,5,5,5,5,5,5,5,5,5,
		6,6,6,6,6,6,6,6,6,6,
		7,7,7,7,7,7,7,7,7,7,
		8,8,8,8,8,8,8,8,8,8,
	};

}

}

#endif
