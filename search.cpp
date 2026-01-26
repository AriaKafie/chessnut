
#include "search.h"

#include <cmath>
#include <iomanip>
#include <thread>

#include "debug.h"
#include "evaluation.h"
#include "movegen.h"
#include "moveordering.h"
#include "position.h"
#include "uci.h"

int reductions[MAX_PLIES];

static struct {
    Move          best_move;
    uint64_t      nodes;
    int           root_delta;
    volatile bool search_cancelled;
    bool          verbose;
} status;

void Search::noverbose() { status.verbose = false; }

void Search::init()
{
    status.verbose = true;

    for (int i = 0; i < MAX_PLIES; i++)
        reductions[i] = int(2954 / 128.0 * std::log(i));

    eval_init();
}

void handle_search_stop(uint64_t thinktime)
{
    if (thinktime)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(thinktime));
        status.search_cancelled = true;
        return;
    }

    std::string in;
    do
        std::getline(std::cin, in);
    while (in != "stop");

    status.search_cancelled = true;
}

int reduction(bool i, int depth, int mn, int delta) {
    int r = reductions[depth] * reductions[mn];
    return r - delta * 764 / status.root_delta + !i * r * 191 / 512 + 1087 - 32 * mn;
}

template<Color SideToMove>
int qsearch(int alpha, int beta)
{
    status.nodes++;

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

template<NodeType NodeT, Color SideToMove>
int search(int alpha, int beta, int depth, bool null_ok, SearchInfo *si)
{
    constexpr bool Root   = NodeT == ROOT;
    constexpr bool PVNode = Root || NodeT == PV;

    status.nodes++;

    if (status.search_cancelled) [[unlikely]]
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
        int eval = -search<NONPV, !SideToMove>(-beta, -beta + 1, depth - R, false, si + 1);
        state_ptr->key ^= Zobrist::Side;

        if (status.search_cancelled) [[unlikely]]
            return 0;

        if (eval >= beta)
            return eval;
    }

    MoveList<SideToMove> moves;

    if (moves.size() == 0)
        return moves.in_check() ? -MATE + si->ply : 0;

    int       best_eval  = -INFINITE;
    Move      best_move  = NO_MOVE;
    Move      ttmove     = TranspositionTable::lookup_move();
    bool      improving  = si->static_ev > (si - 2)->static_ev;
    int       extension  = Root ? 0 : moves.in_check();
    int       move_count = 0;
    BoundType bound_type = UPPER_BOUND;

    moves.sort(ttmove, si);

    for (Move m : moves)
    {
        move_count++;

        if (Root)
            status.root_delta = beta - alpha;

        int eval      = -INFINITE;
        int new_depth = depth - 1 + extension;
        int r         = reduction(improving, depth, move_count, beta - alpha);
        int lmr_depth = new_depth - r / 1024;

        if (!Root && lmr_depth < 7 && type_of(m) == NORMAL)
        {
            int futility_value =
                si->static_ev + piece_weight(piece_type_on(to_sq(m))) + 200 + 116 * lmr_depth;

            if (futility_value <= alpha)
                continue;
        }

        do_move<SideToMove>(m);

        if (depth >= 2 && move_count > 1)
        {
            int reduced = std::clamp(lmr_depth, 1, new_depth);

            eval = -search<NONPV, !SideToMove>(-(alpha + 1), -alpha, reduced, true, si + 1);

            if (eval > alpha && reduced < new_depth)
            {
                bool doDeeperSearch    = eval > (best_eval + 43 + 2 * new_depth);
                bool doShallowerSearch = eval < best_eval + 9;

                new_depth += doDeeperSearch - doShallowerSearch;

                if (new_depth > reduced)
                    eval = -search<NONPV, !SideToMove>(-(alpha + 1), -alpha, new_depth, !null_ok, si + 1);
            }
            else if (eval > alpha && eval < best_eval + 9)
                new_depth--;
        }

        else if (!PVNode || move_count > 1)
        {
            if (!ttmove)
                r += 1156;

            eval = -search<NONPV, !SideToMove>(-(alpha + 1), -alpha,
                new_depth - (r > 3495) - (r > 5510 && new_depth > 2), !null_ok, si + 1);
        }

        if (PVNode && (move_count == 1 || eval > alpha))
        {
            eval = -search<PV, !SideToMove>(-beta, -alpha, new_depth, false, si + 1);
        }

        undo_move<SideToMove>(m);

        if (status.search_cancelled) [[unlikely]]
            return 0;

        if (eval >= beta)
        {
            TranspositionTable::record(depth, LOWER_BOUND, eval, m, si->ply);

            if (is_quiet(m) && m != si->killers[0])
            {
                si->killers[1] = si->killers[0];
                si->killers[0] = m;
            }

            return eval;
        }

        if (eval > alpha)
        {
            best_move = m;
            alpha = eval;
            bound_type = EXACT;

            if (Root)
                status.best_move = best_move;
        }

        best_eval = std::max(eval, best_eval);
    }

    TranspositionTable::record(depth, bound_type, alpha, best_move, si->ply);

    return alpha;
}

template<Color SideToMove>
void iterative_deepening(int max_depth = MAX_DEPTH)
{
    status.best_move = NO_MOVE;
    status.nodes     = 0;

    SearchInfo search_stack[MAX_PLIES] = {}, *si = search_stack + 7;

    for (SearchInfo *s = si + 1; s != search_stack + MAX_PLIES; s++)
        s->ply = (s - 1)->ply + 1;

    uint64_t start   = unix_ms();
    int      window  = 50;
    int guess, alpha = -INFINITE, beta = INFINITE;

    for (int depth = 1; depth <= max_depth; depth++)
    {
        fail:

        int eval = search<ROOT, SideToMove>(alpha, beta, depth, false, si);

        if (status.search_cancelled)
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

        if (status.verbose)
            std::cout << "info depth " << depth
                      << " score cp "  << eval
                      << " nodes "     << status.nodes
                      << " nps "       << status.nodes * 1000 / (std::max(1, int(unix_ms() - start)))
                      << " pv "        << Debug::pv() << std::endl;
    }
}

void Search::go(uint64_t thinktime)
{
    status.search_cancelled = false;

    std::thread t(handle_search_stop, thinktime);
    t.detach();

    Position::white_to_move() ? iterative_deepening<WHITE>()
                              : iterative_deepening<BLACK>();

    while (!status.search_cancelled)
    {}

    std::cout << "bestmove " << move_to_uci(status.best_move) << std::endl;
}

void Search::count_nodes(int depth)
{
    bool   verbose          = status.verbose;
    status.verbose          = false;
    status.search_cancelled = false;

    int maxw = 0;

    for (const std::string& fen : Debug::fens)
        maxw = std::max(maxw, int(fen.size()));

    std::cout << std::left << std::setw(maxw + 1) << "Fen" << "Nodes" << std::endl;

    int total_nodes = 0;

    for (const std::string& fen : Debug::fens)
    {
        std::cout << std::left << std::setw(maxw + 1) << fen;

        Position::set(fen);

        TranspositionTable::clear();
        RepetitionTable::clear();

        Position::white_to_move() ? iterative_deepening<WHITE>(depth)
                                  : iterative_deepening<BLACK>(depth);

        std::cout << status.nodes << std::endl;

        total_nodes += status.nodes;
    }

    std::cout << "total: " << total_nodes << std::endl;
    status.verbose = verbose;
}
