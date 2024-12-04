
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

static Move root_move;
int nodes, reductions[MAX_PLY][MAX_PLY];

void Search::init()
{
    for (int depth = 0; depth < MAX_PLY; depth++)
        for (int move_num = 0; move_num < MAX_PLY; move_num++)
            reductions[depth][move_num] = int(std::log(move_num + 2) / std::log(std::min(14, depth) + 2));
}

template<Color SideToMove>
int qsearch(int alpha, int beta)
{
    nodes++;

    int eval = static_eval<SideToMove>();

    if (eval >= beta)
        return eval;

    alpha = std::max(alpha, eval);

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

        alpha = std::max(alpha, eval);
    }

    return alpha;
}

template<bool Root, Color SideToMove>
int search(int alpha, int beta, int depth, int ply_from_root, bool null_ok)
{
    nodes++;

    if (search_cancelled) [[unlikely]]
        return 0;
        
    if (!Root && RepetitionTable::draw())
        return 0;

    if (depth <= 0)
        return qsearch<SideToMove>(alpha, beta);

    if (!Root)
        if (int lookup = TranspositionTable::lookup(depth, alpha, beta, ply_from_root); lookup != NO_EVAL)
            return lookup;

    if (null_ok && Position::midgame() && depth >= 3 && !Position::in_check<SideToMove>())
    {
        state_ptr->key ^= Zobrist::Side;
        int eval = -search<false, !SideToMove>(-beta, -beta + 1, depth - 3, ply_from_root + 1, false);
        state_ptr->key ^= Zobrist::Side;

        if (search_cancelled) [[unlikely]]
            return 0;

        if (eval >= beta)
            return eval;
    }

    Move      best_move  = NO_MOVE;
    BoundType bound_type = UPPER_BOUND;

    MoveList<SideToMove> moves;

    if (moves.size() == 0)
        return moves.in_check() ? -matescore + ply_from_root : 0;

    moves.sort(TranspositionTable::lookup_move(), ply_from_root);

    int static_evaluation = NO_EVAL, extension = Root ? 0 : moves.in_check();

    for (int i = 0; i < moves.size(); i++)
    {
        int reduction = Root ? root_reductions[i] : reductions[depth][i];

        if (depth - 1 - reduction + extension <= 0 && type_of(moves[i]) == NORMAL && is_quiet(moves[i]))
        {
            if (static_evaluation == NO_EVAL)
                static_evaluation = static_eval<SideToMove>();

            if (static_evaluation + 200 <= alpha)
                continue;
        }

        do_move<SideToMove>(moves[i]);

        int eval = -search<false, !SideToMove>(-beta, -alpha, depth - 1 - reduction + extension, ply_from_root + 1, true);

        if (eval > alpha && reduction && depth - 1 + extension > 0)
            eval = -search<false, !SideToMove>(-beta, -alpha, depth - 1 + extension, ply_from_root + 1, true);
        
        undo_move<SideToMove>(moves[i]);

        if (search_cancelled) [[unlikely]]
            return 0;

        if (eval >= beta)
        {
            TranspositionTable::record(depth, LOWER_BOUND, eval, moves[i], ply_from_root);

            if (is_quiet(moves[i]))
                killers[ply_from_root].add(moves[i] & 0xffff);

            return eval;
        }

        if (eval > alpha)
        {
            best_move = moves[i];
            alpha = eval;
            bound_type = EXACT;

            if (Root)
                root_move = best_move;
        }
    }

    TranspositionTable::record(depth, bound_type, alpha, best_move, ply_from_root);

    return alpha;
}

template<Color SideToMove>
void iterative_deepening(int max_depth = 64)
{
    for (int depth = 1; depth <= max_depth; depth++)
    {
        int eval = search<true, SideToMove>(-INFINITE, INFINITE, depth, 0, false);

        if (search_cancelled)
            break;

        std::cout << "info depth " << depth << " score cp " << eval << " nodes " << nodes << " pv " << Debug::pv() << std::endl;
    }
}

void Search::go(uint64_t thinktime)
{
    search_cancelled = false;
    nodes = 0;

    std::thread t(handle_search_stop, thinktime);
    t.detach();

    if (Position::white_to_move()) iterative_deepening<WHITE>();
    else                           iterative_deepening<BLACK>();

    while (!search_cancelled)
    {}

    std::cout << "bestmove " << move_to_uci(root_move) << std::endl;
}

void Search::count_nodes(int depth)
{
    search_cancelled = false;

    int maxw = 0;

    for (const std::string& fen : Debug::fens)
        maxw = std::max(maxw, int(fen.size()));

    std::cout << std::left << std::setw(maxw + 1) << "Fen" << "Nodes" << std::endl;

    int total_nodes = 0;

    for (const std::string& fen : Debug::fens)
    {
        nodes = 0;

        std::cout << std::left << std::setw(maxw + 1) << fen;

        Position::set(fen);

        clear();
        TranspositionTable::clear();
        RepetitionTable::clear();

        if (Position::white_to_move()) iterative_deepening<WHITE>(depth);
        else                           iterative_deepening<BLACK>(depth);

        std::cout << nodes << std::endl;

        total_nodes += nodes;
    }

    std::cout << "total: " << total_nodes << std::endl;
}
