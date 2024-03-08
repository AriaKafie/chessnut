
#include "board.h"
#include "util.h"
#include "gamestate.h"
#include "zobrist.h"
#include "book.h"
#include "search.h"

void Board::make_legal(Move move) {

  if (!Search::in_search) Book::update(move);
  if (GameState::white_to_move)
    makemove<WHITE, false>(move);
  else 
    makemove<BLACK, false>(move);
  GameState::update(move);

}

void Board::undo_legal(Move move, Piece capture) {

  if (GameState::white_to_move) 
    undomove<BLACK, false>(move, capture);
  else 
    undomove<WHITE, false>(move, capture);
  GameState::restore();

}
