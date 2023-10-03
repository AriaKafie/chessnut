
#ifndef PERFT_H
#define PERFT_H

#include <cstdint>

namespace Chess {

namespace Perft {

	inline uint64_t leafnodes;
	void go(int depth);
	void expand(bool white, int depth);
	int bulk_white();
	int bulk_black();

}

}

#endif
