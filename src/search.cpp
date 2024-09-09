
#include "search.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <thread>

#include "debug.h"
#include "evaluation.h"
#include "movegen.h"
#include "moveordering.h"
#include "position.h"
#include "uci.h"

int nodes, reductions[MAX_PLY][MAX_PLY];

void Search::init() {
    for (int depth = 0; depth < MAX_PLY; depth++)
        for (int move_num = 0; move_num < MAX_PLY; move_num++)
            reductions[depth][move_num] = int(std::log(move_num + 2) / std::log(std::min(14, depth) + 2));
}

template<Color SideToMove>
int qsearch(int alpha, int beta) {

    nodes++;

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
int search(int alpha, int beta, int depth, int ply_from_root, bool null_ok) {

    nodes++;

    if (search_cancelled) [[unlikely]]
        return 0;
        
    if (RepetitionTable::has_repeated())
        return 0;

    if (depth <= 0)
        return qsearch<SideToMove>(alpha, beta);

    if (int lookup = TranspositionTable::lookup(depth, alpha, beta, ply_from_root); lookup != FAIL)
        return lookup;

    if (null_ok && Position::midgame() && depth >= 3 && !Position::in_check<SideToMove>())
    {
        state_ptr->key ^= Zobrist::Side;
        int eval = -search<!SideToMove>(-beta, -beta + 1, depth - 3, ply_from_root + 1, false);
        state_ptr->key ^= Zobrist::Side;

        if (search_cancelled) [[unlikely]]
            return 0;

        if (eval >= beta)
            return eval;
    }

    Move     best_move  = NO_MOVE;
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
void iterative_deepening() {

    Move best_move = TranspositionTable::lookup_move();

    MoveList<SideToMove> moves;

    for (int depth = 1; depth < MAX_PLY && !search_cancelled; depth++)
    {
        int alpha = -INFINITE;

        for (Move& m : moves)
            m &= 0xffff;

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

        std::cout << "info depth " << depth << " score cp " << alpha << " nodes " << nodes << " pv " << move_to_uci(best_move) << "\n";
    }

    std::cout << "bestmove " << move_to_uci(best_move) << "\n";
}

void Search::go(uint64_t thinktime) {

    search_cancelled = false;

    nodes = 0;

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

template<Color SideToMove>
void go_depth(int depth) {

    Move best_move = NO_MOVE;

    for (int d = 1; d <= depth; d++)
    {
        int alpha = -INFINITE;

        MoveList<SideToMove> moves;
        moves.sort(best_move, 0);

        for (int i = 0; i < moves.size(); i++)
        {
            do_move<SideToMove>(moves[i]);

            int eval = -search<!SideToMove>(-INFINITE, -alpha, d - 1 - root_reductions[i], 0, true);

            if (eval > alpha && root_reductions[i])
                eval = -search<!SideToMove>(-INFINITE, -alpha, d - 1, 0, true);

            undo_move<SideToMove>(moves[i]);

            if (eval > alpha)
            {
                alpha = eval;
                best_move = moves[i];
            }
        }
    }
}

void Search::count_nodes(int depth) {

    int maxlen = 0;

    for (const std::string& fen : Debug::fens)
        maxlen = std::max(maxlen, int(fen.size()));

    std::cout << std::left << std::setw(maxlen + 1) << "Fen" << "Nodes" << std::endl;

    int node_sum = 0;

    for (const std::string& fen : Debug::fens)
    {
        nodes = 0;

        std::cout << std::left << std::setw(maxlen + 1) << fen;

        Position::set(fen);

        TranspositionTable::clear();

        if (Position::white_to_move()) go_depth<WHITE>(depth);
        else                           go_depth<BLACK>(depth);

        std::cout << nodes << std::endl;

        node_sum += nodes;
    }

    std::cout << "total: " << node_sum << std::endl;
}
