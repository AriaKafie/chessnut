
#include "zobrist.h"
#include "board.h"
#include "gamestate.h"
#include <random>

namespace Chess {

namespace Zobrist {

	void init() {

		key = 0ull;
		current_seed = 303211888; // 673611206
		//std::mt19937_64 rng(221564671644);
		//std::uniform_int_distribution<uint64_t> distribution;

		for (int i = 0; i < 13; i++) {
			for (int j = 0; j < 64; j++) {
				hash[i][j] = i < 12 ? random_u64() : 0; // thirteenth square state is emptiness
			}
		}

		black_to_move = random_u64();

		if (!GameState::white_to_move)
			key ^= black_to_move;

		for (int square = 0; square < 64; square++)
			key ^= hash[Board::piece_types[square]][square];

	}

	void update() {

		key = 0ull;

		for (int square = 0; square < 64; square++)
			key ^= hash[Board::piece_types[square]][square];
		//if (!GameState::white_to_move)
		//	key ^= black_to_move;

	}

	uint32_t random_u32() {

		uint32_t num = current_seed;

		num ^= num << 13;
		num ^= num >> 17;
		num ^= num << 5;

		current_seed = num;

		return num;

	}

	uint64_t random_u64() {

		uint64_t n1, n2, n3, n4;

		n1 = (uint64_t)(random_u32()) & 0xFFFFull;
		n2 = (uint64_t)(random_u32()) & 0xFFFFull;
		n3 = (uint64_t)(random_u32()) & 0xFFFFull;
		n4 = (uint64_t)(random_u32()) & 0xFFFFull;

		return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);

	}

}

}
