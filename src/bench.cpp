
#include "bench.h"
#include "util.h"
#include "search.h"

#include "position.h"

#include "ui.h"
#include "evaluation.h"
#include "transpositiontable.h"
#include "debug.h"
#include "moveordering.h"
#include "uci.h"

#include <chrono>
#include <iostream>
#include <algorithm>

//void Bench::count_nodes(int depth) {
//
//  nodes = 0;
//  qnodes = 0;
//  int node_sum = 0;
//  int q_node_sum = 0;
//  uint64_t total_time = 0;
//
//  for (std::string fen : fens) {
//
//    nodes = qnodes = 0;
//    std::cout << fen << "  ";
//    Position::set(fen);
//    TranspositionTable::clear();
//    Move best_move = NULLMOVE;
//    auto start_time = curr_time_millis();
//
//    for (int d = 1; d <= depth; d++) {
//      if (Position::white_to_move()) {
//
//        int alpha = -INFINITE;
//        MoveList<WHITE> moves;
//        moves.sort(best_move, 0);
//
//        for (int i = 0; i < moves.size(); i++) {
//          do_move<WHITE>(moves[i]);
//          int eval = Search::search<false>(alpha, INFINITE, d - 1 - reduction[i], 0);
//          if ((eval > alpha) && (reduction[i]))
//            eval = Search::search<false>(alpha, INFINITE, d - 1, 0);
//          if (eval > alpha) {
//            alpha = eval;
//            best_move = moves[i];
//          }
//          undo_move<WHITE>(moves[i]);
//        }
//      }
//      else {
//        int beta = INFINITE;
//        MoveList<BLACK> moves;
//        moves.sort(best_move, 0);
//
//        for (int i = 0; i < moves.size(); i++) {
//          do_move<BLACK>(moves[i]);
//          int eval = Search::search<true>(-INFINITE, beta, d - 1 - reduction[i], 0);
//          if ((eval < beta) && (reduction[i]))
//            eval = Search::search<true>(-INFINITE, beta, d - 1, 0);
//          if (eval < beta) {
//            beta = eval;
//            best_move = moves[i];
//          }
//          undo_move<BLACK>(moves[i]);
//        }
//      }
//    }
//
//    auto duration_ms = curr_time_millis() - start_time;
//    total_time += duration_ms;
//
//    std::cout << nodes << " nodes and " << qnodes << " qnodes\nsearched in " << duration_ms << " ms\n\n";
//    node_sum += nodes;
//    q_node_sum += qnodes;
//
//  }
//  std::cout << "total nodes: " << node_sum << "\ntotal qnodes: " << q_node_sum << "\nin: " << total_time << " ms\n";
//}
