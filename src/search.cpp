
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

int reductions[MAX_DEPTH];

void Search::init()
{
    status.verbose = true;

    for (int i = 0; i < MAX_DEPTH; i++)
        reductions[i] = int(2954 / 128.0 * std::log(i));
}

int reduction(bool i, int depth, int mn, int delta) {
    int r = reductions[depth] * reductions[mn];
    return r - delta * 764 / Search::status.root_delta + !i * r * 191 / 512 + 1087 - 32 * mn;
}

template<Color SideToMove>
int qsearch(int alpha, int beta)
{
    Search::status.nodes++;

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
int search(int alpha, int beta, int depth, bool null_ok, SearchInfo *si)
{
    Search::status.nodes++;

    if (Search::status.search_cancelled) [[unlikely]]
        return 0;
        
    if (!Root && RepetitionTable::draw())
        return 0;

    if (depth <= 0)
        return qsearch<SideToMove>(alpha, beta);

    if (int lookup = TranspositionTable::lookup(depth, alpha, beta, si->ply); !Root && lookup != NO_EVAL)
        return lookup;

    si->static_ev = static_eval<SideToMove>();

    if (null_ok && Position::midgame() && depth >= 3 && !Position::in_check<SideToMove>())
    {
        int R = std::max(1, std::min(int(si->static_ev - beta) / 232, 6) + depth / 3 + 5);

        state_ptr->key ^= Zobrist::Side;
        int eval = -search<false, !SideToMove>(-beta, -beta + 1, depth - R, false, si + 1);
        state_ptr->key ^= Zobrist::Side;

        if (Search::status.search_cancelled) [[unlikely]]
            return 0;

        if (eval >= beta)
            return eval;
    }

    Move      best_move  = NO_MOVE;
    BoundType bound_type = UPPER_BOUND;
    bool      improving  = si->static_ev > (si - 2)->static_ev;

    MoveList<SideToMove> moves;

    if (moves.size() == 0)
        return moves.in_check() ? -matescore + si->ply : 0;

    moves.sort(TranspositionTable::lookup_move(), si->ply);

    int extension = Root ? 0 : moves.in_check();

    for (int i = 0; i < moves.size(); i++)
    {
        if (Root)
            Search::status.root_delta = beta - alpha;

        int new_depth = depth - 1;

        if (depth >= 2 && i > 1)
            new_depth = std::max(1, new_depth - reduction(improving, depth, i, beta - alpha) / 1024);

        if (new_depth <= 2 && type_of(moves[i]) == NORMAL)
        {
            const int depth_scale = 100;

            int margin = 200 + depth_scale * new_depth;

            if (si->static_ev + piece_weight(piece_type_on(to_sq(moves[i]))) + margin <= alpha)
                continue;
        }

        do_move<SideToMove>(moves[i]);

        int eval = -search<false, !SideToMove>(-beta, -alpha, new_depth + extension, true, si + 1);

        if (eval > alpha && new_depth < depth - 1)
            eval = -search<false, !SideToMove>(-beta, -alpha, depth - 1 + extension, true, si + 1);
        
        undo_move<SideToMove>(moves[i]);

        if (Search::status.search_cancelled) [[unlikely]]
            return 0;

        if (eval >= beta)
        {
            TranspositionTable::record(depth, LOWER_BOUND, eval, moves[i], si->ply);

            if (is_quiet(moves[i]))
                killers[si->ply].add(moves[i] & 0xffff);

            return eval;
        }

        if (eval > alpha)
        {
            best_move = moves[i];
            alpha = eval;
            bound_type = EXACT;

            if (Root)
                Search::status.root_move = best_move;
        }
    }

    TranspositionTable::record(depth, bound_type, alpha, best_move, si->ply);

    return alpha;
}

template<Color SideToMove>
void iterative_deepening(int max_depth = 64)
{
    SearchInfo search_stack[MAX_DEPTH] = {}, *si = search_stack + 7;

    for (SearchInfo *s = si + 1; s != search_stack + MAX_DEPTH; s++)
        s->ply = (s - 1)->ply + 1;

    const int window = 50;
    int guess, alpha = -INFINITE, beta = INFINITE;

    uint64_t start = unix_ms();

    for (int depth = 1; depth <= max_depth; depth++)
    {
        fail:

        int eval = search<true, SideToMove>(alpha, beta, depth, false, si);

        if (Search::status.search_cancelled)
            break;

        if (eval <= alpha)
        {
            int margin = guess - alpha;
            alpha = guess - margin * 2;
            goto fail;
        }
        if (eval >= beta)
        {
            int margin = beta - guess;
            beta = guess + margin * 2;
            goto fail;
        }

        guess = eval;

        alpha = eval - window;
        beta  = eval + window;

        if (Search::status.verbose)
            std::cout << "info depth " << depth
                      << " score cp "  << eval
                      << " nodes "     << Search::status.nodes
                      << " time "      << (unix_ms() - start)
                      << " pv "        << Debug::pv() << std::endl;
    }
}

void Search::go(uint64_t thinktime)
{
    Search::status.search_cancelled = false;
    Search::status.nodes = 0;

    std::thread t(handle_search_stop, thinktime);
    t.detach();

    Position::white_to_move() ? iterative_deepening<WHITE>()
                              : iterative_deepening<BLACK>();

    while (!Search::status.search_cancelled)
    {}

    std::cout << "bestmove " << move_to_uci(Search::status.root_move) << std::endl;
}

void Search::count_nodes(int depth)
{
    bool verbose            = status.verbose;
    status.verbose          = false;
    status.search_cancelled = false;

    int maxw = 0;

    for (const std::string& fen : Debug::fens)
        maxw = std::max(maxw, int(fen.size()));

    std::cout << std::left << std::setw(maxw + 1) << "Fen" << "Nodes" << std::endl;

    int total_nodes = 0;

    for (const std::string& fen : Debug::fens)
    {
        Search::status.nodes = 0;

        std::cout << std::left << std::setw(maxw + 1) << fen;

        Position::set(fen);

        clear();
        TranspositionTable::clear();
        RepetitionTable::clear();

        Position::white_to_move() ? iterative_deepening<WHITE>(depth)
                                  : iterative_deepening<BLACK>(depth);

        std::cout << Search::status.nodes << std::endl;

        total_nodes += Search::status.nodes;
    }

    std::cout << "total: " << total_nodes << std::endl;
    status.verbose = verbose;
}
