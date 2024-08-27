
#include "search.h"

#include <cmath>
#include <thread>

#include "bench.h"
#include "debug.h"
#include "evaluation.h"
#include "movegen.h"
#include "moveordering.h"
#include "position.h"
#include "uci.h"
#include "util.h"

int reductions[MAX_PLY][MAX_PLY], nodes, qnodes;

void Search::init() {
    for (int depth = 0; depth < MAX_PLY; depth++)
        for (int mn = 0; mn < MAX_PLY; mn++)
            reductions[depth][mn] = int(std::log(mn + 2) / std::log(std::min(14, depth) + 2));
}

template<Color SideToMove>
int qsearch(int alpha, int beta)
{
    qnodes++;

    int eval = static_eval<SideToMove>();

    if (eval >= beta)
        return beta;

    alpha = std::max(alpha, eval);

    int best_eval = -INFINITE;

    CaptureList<SideToMove> captures;

    if (captures.size() == 0)
        return eval;

    captures.sort();

    for (Move move : captures)
    {
        Piece captured = piece_on(to_sq(move));

        do_capture<SideToMove>(move);
        eval = -qsearch<!SideToMove>(-beta, -alpha);
        undo_capture<SideToMove>(move, captured);

        if (eval >= beta)
            return eval;

        best_eval = std::max(eval, best_eval);

        alpha = std::max(alpha, eval);
    }

    return best_eval;
}

template<Color SideToMove>
int search(int alpha, int beta, int depth, int ply_from_root, bool do_null)
{
    nodes++;

    if (search_cancelled) [[unlikely]]
        return 0;
        
    if (RepetitionTable::has_repeated())
        return 0;

    if (depth <= 0)
        return qsearch<SideToMove>(alpha, beta);

    if (int lookup = TranspositionTable::lookup(depth, alpha, beta, ply_from_root); lookup != FAIL)
        return lookup;

    if (do_null && Position::midgame() && depth >= 3 && !Position::in_check<SideToMove>())
    {
        state_ptr->key ^= Zobrist::Side;
        int eval = -search<!SideToMove>(-beta, -beta + 1, depth - 3, ply_from_root + 1, false);
        state_ptr->key ^= Zobrist::Side;

        if (search_cancelled) [[unlikely]]
            return 0;

        if (eval >= beta)
            return eval;
    }

    Move     best_move  = NULLMOVE;
    HashFlag bound_type = UPPER_BOUND;

    MoveList<SideToMove> moves;

    if (moves.size() == 0)
        return moves.in_check() ? -matescore + ply_from_root : 0;

    moves.sort(TranspositionTable::lookup_move(), ply_from_root);

    int extension = moves.in_check();

    for (int i = 0; i < moves.size(); i++)
    {
        int reduction = reductions[depth][i];

        do_move<SideToMove>(moves[i]);

        int eval = -search<!SideToMove>(-beta, -alpha, depth - 1 - reduction + extension, ply_from_root + 1, true);

        if (eval > alpha && reduction && depth - 1 + extension > 0)
            eval = -search<!SideToMove>(-beta, -alpha, depth - 1 + extension, ply_from_root + 1, true);
        
        undo_move<SideToMove>(moves[i]);

        if (search_cancelled) [[unlikely]]
            return 0;

        if (eval >= beta)
        {
            TranspositionTable::record(depth, LOWER_BOUND, eval, moves[i], ply_from_root);

            if (!piece_on(to_sq(moves[i])))
                killer_moves[ply_from_root].add(moves[i] & 0xffff);

            return eval;
        }

        if (eval > alpha)
        {
            best_move = moves[i];
            alpha = eval;
            bound_type = EXACT;
        }
    }

    TranspositionTable::record(depth, bound_type, alpha, best_move, ply_from_root);

    return alpha;
}

template<Color SideToMove>
void iterative_deepening()
{
    Move best_move = TranspositionTable::lookup_move();

    MoveList<SideToMove> moves;

    for (int depth = 1; depth < MAX_PLY && !search_cancelled; depth++)
    {
        for (Move& m : moves)
            m &= 0xffff;

        int alpha = -INFINITE;

        moves.sort(best_move, 0);

        for (int i = 0; i < moves.size(); i++)
        {
            do_move<SideToMove>(moves[i]);

            int eval = -search<!SideToMove>(-INFINITE, -alpha, depth - 1 - root_reductions[i], 0, true);

            if (eval > alpha && root_reductions[i])
                eval = -search<!SideToMove>(-INFINITE, -alpha, depth - 1, 0, true);

            undo_move<SideToMove>(moves[i]);

            if (search_cancelled)
                break;

            if (eval > alpha)
            {
                alpha = eval;
                best_move = moves[i];
            }
        }

        std::cout << "info depth " << depth << " score cp " << alpha << " pv " << move_to_uci(best_move) << "\n";
    }

    std::cout << "bestmove " << move_to_uci(best_move) << "\n";
}

void Search::go(uint64_t thinktime) {

    search_cancelled = false;

    nodes = qnodes = 0;

    if (thinktime)
    {
        std::thread timer([thinktime]() { start_timer(thinktime); });
        timer.detach();
    }
    else
    {
        std::thread t([]() { await_stop(); });
        t.detach();
    }

    if (Position::white_to_move()) iterative_deepening<WHITE>();
    else                           iterative_deepening<BLACK>();
}

void Bench::count_nodes(int depth) {

    int node_sum = 0;
    int q_node_sum = 0;
    uint64_t total_time = 0;

    for (std::string fen : fens) {

        nodes = qnodes = 0;
        std::cout << fen << "  ";
        Position::set(fen);
        TranspositionTable::clear();
        Move best_move = NULLMOVE;
        auto start_time = curr_time_millis();

        for (int d = 1; d <= depth; d++) {
            if (Position::white_to_move()) {

                int alpha = -INFINITE;
                MoveList<WHITE> moves;
                moves.sort(best_move, 0);

                for (int i = 0; i < moves.size(); i++) {
                    do_move<WHITE>(moves[i]);
                    int eval = -search<BLACK>(-INFINITE, -alpha, d - 1 - root_reductions[i], 0, true);
                    if (eval > alpha && root_reductions[i])
                        eval = -search<BLACK>(-INFINITE, -alpha, d - 1, 0, true);
                    undo_move<WHITE>(moves[i]);
                    if (eval > alpha) {
                        alpha = eval;
                        best_move = moves[i];
                    }
                }
            } else {
                int alpha = -INFINITE;
                MoveList<BLACK> moves;
                moves.sort(best_move, 0);

                for (int i = 0; i < moves.size(); i++) {
                    do_move<BLACK>(moves[i]);
                    int eval = -search<WHITE>(-INFINITE, -alpha, d - 1 - root_reductions[i], 0, true);
                    if (eval > alpha && root_reductions[i])
                        eval = -search<WHITE>(-INFINITE, -alpha, d - 1, 0, true);
                    undo_move<BLACK>(moves[i]);
                    if (eval > alpha) {
                        alpha = eval;
                        best_move = moves[i];
                    }
                }
            }
        }

        auto duration_ms = curr_time_millis() - start_time;
        total_time += duration_ms;

        std::cout << nodes << " nodes and " << qnodes << " qnodes\nsearched in " << duration_ms << " ms\n\n";
        node_sum += nodes;
        q_node_sum += qnodes;

    }
    std::cout << "total nodes: " << node_sum << "\ntotal qnodes: " << q_node_sum << "\nin: " << total_time << " ms\n";
}
