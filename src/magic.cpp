
#include "bitboard.h"
#include "util.h"

#include <vector>
#include <random>
#include <iostream>
#include <fstream>

std::mt19937_64 rng(curr_time_millis());

uint64_t magic_candidate();
bool has_collisions(const std::vector<int>& keys, const std::vector<uint64_t>& values);

/*  PRODUCTIVE COLLISION MAGICS ( >> 65 - bitcount)
        
    rook magics:

    bishop magics:

    01: 0x36e26799bee78bfeull
    02: 0xbe5e8d8b1d47fc56ull
*/

void Magic::test_magic(Square sq, uint64_t magic) {

  std::ofstream o("C:\\Users\\14244\\Desktop\\magic.txt");

  if (!o.is_open()) {
    std::cout << "file open failed";
    std::exit(0);
  }

  for (int p = 0; p < 1 << popcount(rook_masks[sq]); p++) {
    Bitboard occupancy = generate_occupancy(rook_masks[sq], p);
    o << bbtos(occupancy) << (occupancy * magic >> 64 - popcount(rook_masks[sq])) << "\n";
  }

  o.close();
  std::cout << "file write successful";

}

void Magic::search() {

  std::cout << "Bitboard rook_magics[SQUARE_NB] =\n{\n";

  for (Square sq = H1; sq <= A8; sq++) {

    std::vector<Bitboard> occupancies;
    std::vector<Bitboard> attacks;

    int bitcount = popcount(rook_masks[sq]);

    for (int p = 0; p < 1 << bitcount; p++) {
      Bitboard occ = generate_occupancy(rook_masks[sq], p);
      occupancies.push_back(occ);
      attacks.push_back(rook_attacks(sq, occ));
    }

    uint64_t magic;
    std::vector<int> keys;

    do
    {
      magic = magic_candidate();
      keys.clear();
      for (Bitboard occupancy : occupancies)
        keys.push_back(occupancy * magic >> 64 - bitcount);
    }
    while (has_collisions(keys, attacks));

    std::cout << "  0x" << std::hex << magic << "ull,\n";
  }
  std::cout << "};\n";
}

uint64_t magic_candidate() {
  return rng() & rng() & rng();
}

bool has_collisions(const std::vector<int>& keys, const std::vector<uint64_t>& values) {
  for (int i = 0; i < keys.size(); i++) {
    for (int j = 0; j < keys.size(); j++) {
      if (keys[i] == keys[j] && values[i] != values[j])
        return true;
    }
  }
  return false;
}
