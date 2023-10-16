
#ifndef PERFT_H
#define PERFT_H

#include <cstdint>

namespace Chess {

namespace Perft {

	template<bool white>
	void expand(int depth);
	inline uint64_t leafnodes;
	void go(int depth);
	int bulk_white();
	int bulk_black();

}

}

#endif
