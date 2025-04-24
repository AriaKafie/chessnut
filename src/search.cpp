
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

int reductions[MAX_PLIES];

static struct {
    Move     best_move;
    uint64_t nodes;
    int      root_delta;
    bool     search_cancelled;
    bool     verbose;
    std::vector<RootMove> root_moves;
} status;

void Search::noverbose() { status.verbose = false; }

void Search::init()
{
    status.verbose = true;

    for (int i = 0; i < MAX_PLIES; i++)
        reductions[i] = int(2954 / 128.0 * std::log(i));
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

    int       best_eval  = -INFINITE;
    Move      best_move  = NO_MOVE;
    Move      ttmove     = TranspositionTable::lookup_move();
    BoundType bound_type = UPPER_BOUND;
    bool      improving  = si->static_ev > (si - 2)->static_ev;

    MoveList<SideToMove> moves;

    if (moves.size() == 0)
        return moves.in_check() ? -MATE + si->ply : 0;

    moves.sort(ttmove, si->ply);

    int extension  = Root ? 0 : moves.in_check();
    int move_count = 0;

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

        // LMR
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
                    eval = -search<NONPV, !SideToMove>(-(alpha + 1), -alpha, new_depth, true, si + 1);
            }
            else if (eval > alpha && eval < best_eval + 9)
                new_depth--;
        }

        // NO LMR
        else if (!PVNode || move_count > 1)
        {
            if (!ttmove)
                r += 1156;

            eval = -search<NONPV, !SideToMove>(-(alpha + 1), -alpha,
                new_depth - (r > 3495) - (r > 5510 && new_depth > 2), true, si + 1);
        }

        // PV OR PV FAIL HIGH
        if (PVNode && (move_count == 1 || eval > alpha))
        {
            eval = -search<PV, !SideToMove>(-beta, -alpha, new_depth, true, si + 1);
        }

        undo_move<SideToMove>(m);

        if (status.search_cancelled) [[unlikely]]
            return 0;

        if (Root)
        {
            RootMove& rm =
                *std::find(status.root_moves.begin(), status.root_moves.end(), m);

            rm.average_score =
                rm.average_score != -INFINITE ? (eval + rm.average_score) / 2 : eval;

            rm.mean_squared_score = rm.mean_squared_score != -INFINITE * INFINITE
                ? (eval * std::abs(eval) + rm.mean_squared_score) / 2
                : eval * std::abs(eval);

            if (move_count == 1 || eval > alpha)
                rm.score = eval;
            else
                rm.score = -INFINITE;
        }

        if (eval >= beta)
        {
            TranspositionTable::record(depth, LOWER_BOUND, eval, m, si->ply);

            if (is_quiet(m))
                killers[si->ply].add(m & 0xffff);

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
    status.root_moves.clear();

    {
        MoveList<SideToMove> moves;
        moves.sort(TranspositionTable::lookup_move(), 0);

        for (Move m : moves)
            status.root_moves.push_back({m, -INFINITE, -INFINITE, -INFINITE, -INFINITE * INFINITE});
    }

    SearchInfo search_stack[MAX_PLIES] = {}, *si = search_stack + 7;

    for (SearchInfo *s = si + 1; s != search_stack + MAX_PLIES; s++)
        s->ply = (s - 1)->ply + 1;

    uint64_t start     = unix_ms();
    int      bestValue = -INFINITE;
    /*int      window    = 50;
    int guess, alpha   = -INFINITE, beta = INFINITE;*/

    for (int depth = 1; depth <= max_depth; depth++)
    {
        for (RootMove& rm : status.root_moves)
            rm.previous_score = rm.score;

        int delta = 5 + std::abs(status.root_moves[0].mean_squared_score) / 11834;
        int avg   = status.root_moves[0].average_score;
        int alpha = std::max(avg - delta, -INFINITE);
        int beta  = std::min(avg + delta, INFINITE);

        int failedHighCnt = 0;

        for (;;)
        {
            int adjustedDepth = std::max(1, depth - failedHighCnt);
            status.root_delta = beta - alpha;

            bestValue = search<ROOT, SideToMove>(alpha, beta, adjustedDepth, false, si);

            std::stable_sort(status.root_moves.begin(), status.root_moves.end());

            if (status.search_cancelled)
                break;

            if (bestValue <= alpha)
            {
                beta  = (alpha + beta) / 2;
                alpha = std::max(bestValue - delta, -INFINITE);

                failedHighCnt = 0;
            }
            else if (bestValue >= beta)
            {
                beta = std::min(bestValue + delta, INFINITE);
                ++failedHighCnt;
            }
            else
                break;

            delta += delta / 3;
        }

        std::stable_sort(status.root_moves.begin(), status.root_moves.end());

        if (status.verbose)
            std::cout << "info depth " << depth
                      << " score cp "  << bestValue
                      << " nodes "     << status.nodes
                      << " nps "       << (status.nodes * 1000 / (unix_ms() - start))
                      << " pv "        << Debug::pv() << std::endl;

        if (status.search_cancelled) break;



        /*fail:

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
                      << " nps "       << (status.nodes * 1000 / (unix_ms() - start))
                      << " pv "        << Debug::pv() << std::endl;*/
    }
}

void Search::go(uint64_t thinktime)
{
    status.search_cancelled = false;
    status.nodes = 0;

    std::thread t(handle_search_stop, thinktime);
    t.detach();

    Position::white_to_move() ? iterative_deepening<WHITE>()
                              : iterative_deepening<BLACK>();

    while (!status.search_cancelled)
    {}

    std::cout << "bestmove " << move_to_uci(
        status.best_move ? status.best_move : TranspositionTable::lookup_move()) << std::endl;
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

        clear();
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
