
struct StateInfo {
  uint64_t key;
  Piece    captured;
  Square   ep_square;
  uint8_t  castling;
}

StateInfo ss[128]; // state stack
int sp;            // stack pointer

void GameState::push() { stack_pointer++; }
void GameState::pop () { stack_pointer--; }

uint64_t GameState::key() { return ss[sp].key; }

Piece GameState::captured() { return ss[sp].captured; }

Square GameState::ep_bb() { return 1ull << ss[sp].ep_square; }

uint8_t GameState::castling() { return ss[sp].castling; }

void do_fancy(Move m) {
  if (side_to_move == WHITE)
    do_move<WHITE>(m);
  else
    do_move<BLACK>(m);
  ss[sp - 1] = ss[sp];
  sp--;
  update_gamephase();
  
}

template<Color Us>
void do_move(Move m) {
  
  GameState::push();

  side_to_move = !side_to_move;
  
  Square from = from_sq(m);
  Square to   = to_sq(m);

  ss[sp].captured = piece_on(to);

  ss[sp].key ^= Zobrist::hash[piece_on(from)][from];
  ss[sp].key ^= Zobrist::hash[piece_on(from)][to];
  
  constexpr Direction Up  = Us == WHITE ? NORTH : SOUTH;
  constexpr Direction Up2 = Up * 2;
  
  ss[sp].ep_square = piece_type_on(from) == PAWN && to - from == Up2 ? from + Up : 0;
  
}

template<Color Us>
void undo_move(Move m) {

  Piece captured = ss[sp].captured;
  
  GameState::pop();
  
}
