
#include "position.h"

#include <cstring>
#include <iomanip>
#include <random>
#include <sstream>

#include "transpositiontable.h"
#include "uci.h"

void set_gamephase();

Bitboard bitboards[16];
Piece board[SQUARE_NB];

StateInfo state_stack[MAX_PLY], *state_ptr = state_stack;

void Position::init()
{
    std::mt19937_64 rng(221564671644);

    for (Piece pc : { W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
                      B_PAWN, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING })
    {
        for (Square sq = H1; sq <= A8; sq++)
            Zobrist::hash[pc][sq] = rng();
    }
}

void Position::set(const std::string& fen)
{
    const std::string piece_to_char = "  PNBRQK  pnbrqk";

    memset(board, NO_PIECE, 64 * sizeof(Piece));
    memset(bitboards, 0ull, 16 * sizeof(Bitboard));

    Square             sq = A8;
    std::istringstream is(fen);
    std::string        pieces, color, castling, enpassant;

    is >> pieces >> color >> castling >> enpassant;

    for (char token : pieces)
    {
        if (std::isdigit(token))
        {
            sq -= token - '0'; 
        }
        else if (size_t piece = piece_to_char.find(token); piece != std::string::npos)
        {
            board[sq] = piece;
            bitboards[piece] |= square_bb(sq);
            bitboards[color_of(piece)] |= square_bb(sq);
            sq--;
        }
    }

    state_ptr->side_to_move = color == "w" ? WHITE : BLACK;

    state_ptr->castling_rights = state_ptr->ep_sq = 0;

    for (char token : castling)
        if (size_t idx = std::string("qkQK").find(token); idx != std::string::npos)
            state_ptr->castling_rights |= 1 << idx;

    if (enpassant != "-")
        state_ptr->ep_sq = uci_to_square(enpassant);

    state_ptr->key = state_ptr->side_to_move == WHITE ? 0 : Zobrist::Side;

    for (Square sq = H1; sq <= A8; sq++)
        state_ptr->key ^= Zobrist::hash[piece_on(sq)][sq];

    RepetitionTable::clear();
    RepetitionTable::push();

    set_gamephase();
}

std::string Position::to_string()
{
    std::stringstream ss;

    ss << "\n+---+---+---+---+---+---+---+---+\n";

    for (Square sq = A8; sq >= H1; sq--)
    {
        ss << "| " << "  PNBRQK  pnbrqk"[board[sq]] << " ";

        if (sq % 8 == 0)
            ss << "| " << (sq / 8 + 1) << "\n+---+---+---+---+---+---+---+---+\n";
    }

    ss << "  a   b   c   d   e   f   g   h\n\nFen: " << fen() << "\nKey: " << std::setw(16) << std::setfill('0') << std::hex << std::uppercase << key() << "\n";

    return ss.str();
}

std::string Position::fen()
{
    std::stringstream fen;

    for (Rank r = RANK_8; r >= RANK_1; r--)
    {
        for (File f = FILE_A; f >= FILE_H; f--)
        {
            if (Piece pc = piece_on(r * 8 + f))
            {
                fen << "  PNBRQK  pnbrqk"[pc];
            }
            else
            {
                int empty = 0;

                for (;f >= FILE_H && !piece_on(r * 8 + f); f--)
                    empty++;

                fen << empty;

                f++;
            }
        }

        if (r) fen << "/";
    }

    fen << " " << "wb"[side_to_move()] << " ";

    if (!state_ptr->castling_rights)
        fen << "-";
    else
    {
        if (state_ptr->castling_rights & 8) fen << "K";
        if (state_ptr->castling_rights & 4) fen << "Q";
        if (state_ptr->castling_rights & 2) fen << "k";
        if (state_ptr->castling_rights & 1) fen << "q";
    }

    fen << " " << (state_ptr->ep_sq ? square_to_uci(state_ptr->ep_sq) : "-");

    return fen.str();
}

void Position::commit_move(Move m)
{
    if (is_capture(m) || piece_type_on(from_sq(m)) == PAWN || type_of(m) != NORMAL)
        RepetitionTable::clear();

    if (white_to_move()) do_move<WHITE>(m);
    else                 do_move<BLACK>(m);

    memcpy(state_stack, state_ptr, sizeof(StateInfo));
    state_ptr = state_stack;
    state_ptr->side_to_move = !state_ptr->side_to_move;

    set_gamephase();
}

void set_gamephase()
{
    Color us = state_ptr->side_to_move, them = !us;

#define piece_count(pc) popcount(bitboards[pc])

    int friendly_material = 3 * piece_count(make_piece(us, KNIGHT)) +
                            3 * piece_count(make_piece(us, BISHOP)) +
                            5 * piece_count(make_piece(us, ROOK))   +
                            9 * piece_count(make_piece(us, QUEEN));
        
    int enemy_material    = 3 * piece_count(make_piece(them, KNIGHT)) +
                            3 * piece_count(make_piece(them, BISHOP)) +
                            5 * piece_count(make_piece(them, ROOK))   +
                            9 * piece_count(make_piece(them, QUEEN));

    bool pawns = bitboards[W_PAWN] || bitboards[B_PAWN];

    if (!pawns && (enemy_material < 5 && friendly_material >= 5 || friendly_material < 5 && enemy_material >= 5))
        state_ptr->gamephase = MOPUP;
    else if (friendly_material < 10 || friendly_material < 17 && !bitboards[make_piece(us, QUEEN)])
        state_ptr->gamephase = ENDGAME;
    else
        state_ptr->gamephase = MIDGAME;
        
#undef piece_count
}

