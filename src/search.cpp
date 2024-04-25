
#include "search.h"

#include "debug.h"
#include "moveordering.h"
#include "movegen.h"
#include "position.h"
#include "uci.h"
#include "ui.h"
#include "util.h"

#include <thread>

void poll(uint64_t thinktime) {
  auto start_time = curr_time_millis();
  while (true) {
    if (curr_time_millis() - start_time > thinktime) {
      Search::search_cancelled = true;
      return;
     }
  }
}

Move Search::probe_white(uint64_t thinktime) {

  search_cancelled = false;
  int eval, alpha;
  Move best_move = NULLMOVE;
  MoveList<WHITE> moves;
  
  std::thread timer([thinktime]() { poll(thinktime); });
  timer.detach();

  for (int depth = 1; depth < MAX_PLY && !search_cancelled; depth++)
  {
    for (Move& m : moves)
      m &= 0xffff;

    alpha = -INFINITE;
    moves.sort(best_move, 0);

    for (int i = 0; i < moves.size(); i++) {

      do_move<WHITE>(moves[i]);
      eval = search<false>(alpha, INFINITE, depth - 1 - reduction[i], 0);
      if ((eval > alpha) && reduction[i])
        eval = search<false>(alpha, INFINITE, depth - 1, 0);
      undo_move<WHITE>(moves[i]);

      if (search_cancelled) {
        Debug::last_depth_searched = depth - 1;
        break;
      }

      if (eval > alpha) {
        alpha = eval;
        best_move = moves[i];
      }

    }
  }
  return best_move == NULLMOVE ? moves[0] : best_move;
}

Move Search::probe_black(uint64_t thinktime) {

  search_cancelled = false;
  int eval, beta;
  Move best_move = NULLMOVE;
  MoveList<BLACK> moves;
  
  std::thread timer([thinktime]() { poll(thinktime); });
  timer.detach();

  for (int depth = 1; depth < MAX_PLY && !search_cancelled; depth++)
  {
    for (Move& m : moves)
      m &= 0xffff;

    beta = INFINITE;
    moves.sort(best_move, 0);

    for (int i = 0; i < moves.size(); i++) {

      do_move<BLACK>(moves[i]);
      eval = search<true>(-INFINITE, beta, depth - 1 - reduction[i], 0);
      if ((eval < beta) && reduction[i])
        eval = search<true>(-INFINITE, beta, depth - 1, 0);
      undo_move<BLACK>(moves[i]);

      if (search_cancelled) {
        Debug::last_depth_searched = depth - 1;
        break;
      }

      if (eval < beta) {
        beta = eval;
        best_move = moves[i];
      }

    }
  }
  return best_move == NULLMOVE ? moves[0] : best_move;
}
