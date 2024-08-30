
#include "uci.h"

#include <algorithm>
#include <iomanip>
#include <cctype>
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>
#include <regex>

#include "util.h"
#include "position.h"
#include "transpositiontable.h"
#include "search.h"

#include "debug.h"

void position(std::istringstream& cmd) {

    std::string fen, token;

    cmd >> token;

    if (token == "startpos")
    {
        fen = STARTPOS;
        cmd >> token;
    }
    else if (token == "fen")
    {
        while (cmd >> token && token != "moves")
            fen += token + " ";
    }
    else
        return;

    Position::set(fen);

    while (cmd >> token)
        Position::do_commit(uci_to_move(token));
}

void moves(std::istringstream& cmd) {

    std::string token;

    while (cmd >> token)
        Position::do_commit(uci_to_move(token));
}

void go(std::istringstream& args) {

    int depth;

    std::string token;
    args >> token;

    if (token == "nodes")
    {
        args >> depth;
        Search::count_nodes(depth);
    } 
    else if (token == "debug")
    {
        Debug::go();
    }
    else if (token == "movetime")
    {
        uint64_t thinktime;
        args >> thinktime;
        Search::go(thinktime);
    }
    else
        Search::go();
}

void UCI::loop()
{
    Position::set(STARTPOS);

    std::string cmd, token;

    do
    {
        std::getline(std::cin, cmd);

        std::istringstream ss(cmd);
        ss >> token;

        if      (token == "uci")     std::cout << "uciok"               << std::endl;
        else if (token == "isready") std::cout << "readyok"             << std::endl;
        else if (token == "d")       std::cout << Position::to_string() << std::endl;

        else if (token == "ucinewgame")
        {
            TranspositionTable::clear();
            RepetitionTable::clear();
        }

        else if (token == "perft")
            Debug::perft(ss);
        else if (token == "position")
            position(ss);
        else if (token == "go")
            go(ss);
        else if (token == "moves")
            moves(ss);
        else if (token == "gameinfo")
            Debug::gameinfo();

    } while (cmd != "quit");
}

Move uci_to_move(const std::string& uci) {

  Square from = uci_to_square(uci.substr(0, 2));
  Square to   = uci_to_square(uci.substr(2, 2));

  bool castling  = piece_type_on(from) == KING && std::regex_match(uci, std::regex("e[18][cg][18]"));
  bool promotion = uci.find('q') != std::string::npos;
  bool enpassant = piece_type_on(from) == PAWN && file_bb(to) != file_bb(from) && !piece_on(to);

  if (castling)
    return Position::white_to_move() ? file_bb(to) & FILE_G ? W_SCASTLE : W_LCASTLE : file_bb(to) & FILE_G ? B_SCASTLE : B_LCASTLE;

  return promotion ? make_move<PROMOTION>(from, to) : enpassant ? make_move<ENPASSANT>(from, to) : make_move(from, to);
}

std::string move_to_uci(Move m) {
  return square_to_uci(from_sq(m)) + square_to_uci(to_sq(m)) + (type_of(m) == PROMOTION ? "q" : "");
}
