
#include "search.h"
#include "board.h"
#include "ui.h"
#include "evaluation.h"
#include "util.h"
#include "gamestate.h"
#include "transpositiontable.h"
#include "debug.h"
#include "bench.h"
#include "moveordering.hpp"
#include "movegen.h"
#include "uci.h"
#include "book.h"

#include <iostream>

namespace Search {

Move probe_white(uint64_t thinktime) {

  in_search = true;
  search_cancelled = false;
  int eval;
  int alpha;
  int best_move = NULLMOVE;
  MoveList<WHITE> moves;
  auto start_time = Util::curr_time_millis();

  for (int depth = 1; !search_cancelled; depth++) {

    for (Move& m : moves)
      m &= 0x0000ffff;
    alpha = MIN_INT;
    moves.sort(best_move, 0);

    for (int i = 0; i < moves.length(); i++) {

      if ((Util::curr_time_millis() - start_time) > thinktime) {
        search_cancelled = true;
        Debug::last_depth_searched = depth - 1;
        break;
      }

      Piece capture = Board::pieces[to_sq(moves[i])];
      uint8_t c_rights = GameState::castling_rights;
      Board::make_legal(moves[i]);

      if (GameState::repetition_table.count_last_hash() > 2)
        eval = 0;
      else if (GameState::repetition_table.opponent_can_repeat())
        eval = std::min(search<false>(alpha, MAX_INT, depth - 1, 0), 0);
      else
        eval = search<false>(alpha, MAX_INT, depth - 1 - reduction[i], 0);
      if ((eval > alpha) && reduction[i])
        eval = search<false>(alpha, MAX_INT, depth - 1, 0);

      Board::undo_legal(moves[i], capture);
      GameState::castling_rights = c_rights;

      if (eval > alpha) {
        alpha = eval;
        best_move = moves[i];
      }
    }
  }

  in_search = false;
  search_cancelled = false;
  return best_move == NULLMOVE ? moves[0] : best_move;

}

Move probe_black(uint64_t thinktime) {

  in_search = true;
  search_cancelled = false;
  int eval;
  int beta;
  int best_move = NULLMOVE;
  MoveList<BLACK> moves;
  auto start_time = Util::curr_time_millis();

  for (int depth = 1; !search_cancelled; depth++) {

    for (Move& m : moves)
      m &= 0x0000ffff;
    beta = MAX_INT;
    moves.sort(best_move, 0);

    for (int i = 0; i < moves.length(); i++) {

      if ((Util::curr_time_millis() - start_time) > thinktime) {
        search_cancelled = true;
        Debug::last_depth_searched = depth - 1;
        break;
      }

      Piece capture = Board::pieces[to_sq(moves[i])];
      uint8_t c_rights = GameState::castling_rights;
      Board::make_legal(moves[i]);

      if (GameState::repetition_table.count_last_hash() > 2)
        eval = 0;
      else if (GameState::repetition_table.opponent_can_repeat())
        eval = std::max(search<true>(MIN_INT, beta, depth - 1, 0), 0);
      else {
        eval = search<true>(MIN_INT, beta, depth - 1 - reduction[i], 0);
        if ((eval < beta) && reduction[i])
          eval = search<true>(MIN_INT, beta, depth - 1, 0);
      }

      Board::undo_legal(moves[i], capture);
      GameState::castling_rights = c_rights;

      if (eval < beta) {
        beta = eval;
        best_move = moves[i];
      }
    }
  }

  in_search = false;
  search_cancelled = false;
  return best_move == NULLMOVE ? moves[0] : best_move;

}

} // namespace Search
