
#include "magic.h"
#include "bitboard.h"
#include "util.h"

#include <vector>
#include <random>
#include <iostream>
#include <fstream>

uint64_t random_u64();
uint64_t magic_candidate();
bool has_collisions(const std::vector<int>& hashcodes, const std::vector<uint64_t>& values);

/*  PRODUCTIVE COLLISION MAGICS ( >> 65 - bitcount)
	
	rook magics:

	bishop magics:

	01: 0x36e26799bee78bfeull
	02: 0xbe5e8d8b1d47fc56ull
*/

void thing() {

	std::ofstream o("C:\\Users\\14244\\Desktop\\magic.txt");
	if (!o.is_open()) {
		std::cout << "file open failed";
		std::exit(0);
	}

	const uint64_t magic = 0xa000a104004092ull;
	const uint64_t mask = rook_masks[A8];
	int bitcount = popcount(mask);
	int permutations = 1 << bitcount;

	for (int p = 0; p < permutations; p++) {
		Bitboard occupancy = generate_occupancy(mask, p);
		o << bitboard_to_string(occupancy) << ((occupancy * magic) >> (64 - bitcount)) << "\n";
	}
	o.close();
	std::cout << "file write successful";

}

void Magic::search() {

	//thing(); return;
	constexpr uint64_t search_time = 120000;
	std::cout << "Bitboard rook_magics[SQUARE_NB] = {\n";
	for (Square sq = H1; sq <= A8; sq++) {

		std::vector<Bitboard> occupancies;
		std::vector<Bitboard> attacks;

		Bitboard mask = rook_masks[sq];
		int bitcount = popcount(mask);
		int permutations = 1 << bitcount;
		for (int p = 0; p < permutations; p++) {
			Bitboard occ = generate_occupancy(mask, p);
			occupancies.push_back(occ);
			attacks.push_back(RookAttacks(sq, occ));
		}

		uint64_t magic;
		bool valid_magic = false;

		using Util::curr_time_millis;
		auto start = curr_time_millis();

		while (!valid_magic) {
			magic = magic_candidate();
			std::vector<int> hashcodes;
			for (Bitboard occupancy : occupancies) {
				hashcodes.push_back((occupancy * magic) >> (64 - bitcount));
			}
			valid_magic = !has_collisions(hashcodes, attacks);
		}
		std::cout << "0x" << std::hex << magic << "ull,\n";
	}
	std::cout << "};\n";
}

uint64_t magic_candidate() {
	return random_u64() & random_u64() & random_u64();
}

uint64_t random_u64() {

	std::random_device rd;
	std::mt19937_64 gen(rd());

	std::uniform_int_distribution<uint64_t> dis(std::numeric_limits<uint64_t>::min(),
		std::numeric_limits<uint64_t>::max());

	return dis(gen);
}

bool has_collisions(const std::vector<int>& hashcodes, const std::vector<uint64_t>& values) {
	for (int i = 0; i < hashcodes.size(); i++) {
		for (int j = 0; j < hashcodes.size(); j++) {
			if ((hashcodes[i] == hashcodes[j])
				&& (values[i] != values   [j])) {
				// collision only if: hashcodes are equal and values arent
				return true;
			}
		}
	}
	return false;
}
