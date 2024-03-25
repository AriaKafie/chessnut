
#include "bitboard.h"
#include "util.h"

#include <vector>
#include <random>
#include <iostream>
#include <fstream>
#include <unordered_set>

std::mt19937_64 rng(curr_time_millis());

bool has_duplicates(const std::vector<int>& keys);

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

uint64_t magic_candidate() { return rng() & rng() & rng(); }

void Magic::search() {

  std::cout << "Bitboard rook_magics[SQUARE_NB] =\n{\n";

  for (Square sq = H1; sq <= A8; sq++) {

    int bitcount = popcount(rook_masks[sq]);

    std::vector<Bitboard> occupancies;

    for (int p = 0; p < 1 << bitcount; p++)
      occupancies.push_back(generate_occupancy(rook_masks[sq], p));

    uint64_t magic;
    std::vector<int> keys;

    do
    {
      keys.clear();
      magic = magic_candidate();
      for (Bitboard occupancy : occupancies)
        keys.push_back(occupancy * magic >> 64 - bitcount);
    } while (has_duplicates(keys));

    std::cout << "  0x" << std::hex << magic << "ull,\n";
  }
  std::cout << "};\n";
}

bool has_duplicates(const std::vector<int>& keys) {
  for (int i = 0; i < keys.size(); i++)
    for (int j = i + 1; j < keys.size(); j++)
      if (keys[i] == keys[j])
        return true;
  return false;
}
