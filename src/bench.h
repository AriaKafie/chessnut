
#ifndef BENCH_H
#define BENCH_H

#include <cstdint>

namespace Chess {

namespace Bench {

	inline double timesum;
	inline int node_count;

	void count_nodes(int depth);
	void warmup();
	void go(int runs, uint64_t iterations);

}

}

#endif
