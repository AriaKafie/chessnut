
#include "search.h"
#include "position.h"
#include "ui.h"
#include "util.h"
#include "debug.h"
#include "moveordering.h"
#include "movegen.h"

namespace Search {

Move probe_white(uint64_t thinktime) {

  int eval, alpha;
  Move best_move = NULLMOVE;
  MoveList<WHITE> moves;
  auto start_time = curr_time_millis();

  for (int depth = 1, search_cancelled = false; !search_cancelled; depth++)
  {
    for (Move& m : moves)
      m &= 0xffff;

    alpha = MIN_INT;
    moves.sort(best_move, 0);

    for (int i = 0; i < moves.length(); i++) {

      if ((curr_time_millis() - start_time) > thinktime) {
        search_cancelled = true;
        Debug::last_depth_searched = depth - 1;
        break;
      }

      do_move<WHITE>(moves[i]);
      eval = search<false>(alpha, MAX_INT, depth - 1 - reduction[i], 0);
      if ((eval > alpha) && reduction[i])
        eval = search<false>(alpha, MAX_INT, depth - 1, 0);
      undo_move<WHITE>(moves[i]);

      if (eval > alpha) {
        alpha = eval;
        best_move = moves[i];
      }

    }
  }
  return best_move == NULLMOVE ? moves[0] : best_move;
}

Move probe_black(uint64_t thinktime) {

  int eval, beta;
  Move best_move = NULLMOVE;
  MoveList<BLACK> moves;
  auto start_time = curr_time_millis();

  for (int depth = 1, search_cancelled = false; !search_cancelled; depth++)
  {
    for (Move& m : moves)
      m &= 0xffff;

    beta = MAX_INT;
    moves.sort(best_move, 0);

    for (int i = 0; i < moves.length(); i++) {

      if ((curr_time_millis() - start_time) > thinktime) {
        search_cancelled = true;
        Debug::last_depth_searched = depth - 1;
        break;
      }

      do_move<BLACK>(moves[i]);
      eval = search<true>(MIN_INT, beta, depth - 1 - reduction[i], 0);
      if ((eval < beta) && reduction[i])
        eval = search<true>(MIN_INT, beta, depth - 1, 0);
      undo_move<BLACK>(moves[i]);
     
      if (eval < beta) {
        beta = eval;
        best_move = moves[i];
      }

    }
  }
  return best_move == NULLMOVE ? moves[0] : best_move;
}

} // namespace Search
