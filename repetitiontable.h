
#ifndef REPETITIONTABLE_H
#define REPETITIONTABLE_H

#include <cstdint>

namespace Chess {

class RepetitionTable {

public:
	RepetitionTable();
	bool opponent_can_repeat();
	int count_last_hash();
	void push();
	void pop();
	void print();
	void reset();
private:
	static constexpr int listlength = 128;
	int list_pointer;
	uint64_t hashlist[128];
	uint64_t current_hash();
	int occurrences(uint64_t hash);
	void verify_capacity();

};

}

#endif
