
#include "bench.h"
#include "util.h"
#include "search.h"
#include "repetitiontable.h"
#include "board.h"
#include "gamestate.h"
#include "ui.h"
#include "evaluation.h"
#include "transpositiontable.h"
#include "debug.h"
#include "moveordering.h"
#include "uci.h"

#include <chrono>
#include <iostream>
#include <algorithm>

void Bench::count_nodes(int depth) {

  nodes = 0;
  qnodes = 0;
  int node_sum = 0;
  int q_node_sum = 0;
  uint64_t total_time = 0;

  for (std::string fen : fens) {

    nodes = 0;
    qnodes = 0;
    std::cout << fen << "  ";
    GameState::init(fen);
    TranspositionTable::clear();
    Search::in_search = true;
    Move best_move = NULLMOVE;
    auto start_time = Util::curr_time_millis();

    for (int d = 1; d <= depth; d++) {
      if (GameState::white_to_move) {

        int alpha = MIN_INT;
        MoveList<WHITE> ml(true);
        ml.sort(best_move, 0);

        for (int i = 0; i < ml.length(); i++) {
          uint8_t c_rights = GameState::castling_rights;
          Piece captured = piece_on(to_sq(ml[i]));
          do_legal(ml[i]);
          int eval = Search::search<false>(alpha, MAX_INT, d - 1 - reduction[i], 0);
          if ((eval > alpha) && (reduction[i]))
            eval = Search::search<false>(alpha, MAX_INT, d - 1, 0);
          if (eval > alpha) {
            alpha = eval;
            best_move = ml[i];
          }
          undo_legal(ml[i], captured);
          GameState::castling_rights = c_rights;
        }
      }
      else {
        int beta = MAX_INT;
        MoveList<BLACK> ml(true);
        ml.sort(best_move, 0);

        for (int i = 0; i < ml.length(); i++) {
          Piece captured = piece_on(to_sq(ml[i]));
          uint8_t c_rights = GameState::castling_rights;
          do_legal(ml[i]);
          int eval = Search::search<true>(MIN_INT, beta, d - 1 - reduction[i], 0);
          if ((eval < beta) && (reduction[i]))
            eval = Search::search<true>(MIN_INT, beta, d - 1, 0);
          if (eval < beta) {
            beta = eval;
            best_move = ml[i];
          }
          undo_legal(ml[i], captured);
          GameState::castling_rights = c_rights;
        }
      }
    }

    auto duration_ms = Util::curr_time_millis() - start_time;
    total_time += duration_ms;

    Search::in_search = false;
    std::cout << nodes << " nodes and " << qnodes << " qnodes\nsearched in " << duration_ms << " ms\n\n";
    node_sum += nodes;
    q_node_sum += qnodes;

  }
  std::cout << "total nodes: " << node_sum << "\ntotal qnodes: " << q_node_sum << "\nin: " << total_time << " ms\n";
}
