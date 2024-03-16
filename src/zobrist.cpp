
#include "board.h"
#include "zobrist.h"

#include <random>

uint64_t seed = 221564671644;

void Zobrist::init() {

  key = 0;

  std::mt19937_64 rng(seed);

  for (Piece pc : { W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
                    B_PAWN, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING } )
  {
    for (Square sq = H1; sq <= A8; sq++)
      hash[pc][sq] = rng();
  }

  if (!GameState::white_to_move)
    key ^= BlackToMove;

  for (Square sq = H1; sq <= A8; sq++)
    key ^= hash[piece_on(sq)][sq];

}

void Zobrist::update() {

  key = 0;

  for (int square = 0; square < 64; square++)
    key ^= hash[piece_on(square)][square];

  if (!GameState::white_to_move)
    key ^= BlackToMove;

}