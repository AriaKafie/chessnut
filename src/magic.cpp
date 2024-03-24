
#include "magic.h"
#include "bitboard.h"
#include "util.h"

#include <vector>
#include <random>
#include <iostream>
#include <fstream>
#include <time.h>


std::mt19937_64 rng(time(NULL));

uint64_t magic_candidate();
bool has_collisions(const std::vector<int>& keys, const std::vector<uint64_t>& values);

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
    o << bbtos(occupancy) << ((occupancy * magic) >> (64 - bitcount)) << "\n";
  }
  o.close();
  std::cout << "file write successful";

}

void Magic::search() {

  //thing(); return;
  constexpr uint64_t search_time = 120000;
  std::cout << "Bitboard rook_magics[SQUARE_NB] =\n{\n";
  for (Square sq = H1; sq <= A8; sq++) {

    std::vector<Bitboard> occupancies;
    std::vector<Bitboard> attacks;

    Bitboard mask = rook_masks[sq];
    int bitcount = popcount(mask);
    int permutations = 1 << bitcount;
    for (int p = 0; p < permutations; p++) {
      Bitboard occ = generate_occupancy(mask, p);
      occupancies.push_back(occ);
      attacks.push_back(rook_attacks(sq, occ));
    }

    uint64_t magic;
    bool valid_magic = false;

    using Util::curr_time_millis;
    auto start = curr_time_millis();

    while (!valid_magic) {
      magic = magic_candidate();
      std::vector<int> keys;
      for (Bitboard occupancy : occupancies)
        keys.push_back(occupancy * magic >> 64 - bitcount);
      valid_magic = !has_collisions(keys, attacks);
    }
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
