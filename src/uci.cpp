
#include "uci.h"

#include <iostream>
#include <sstream>

#include "debug.h"
#include "movegen.h"
#include "position.h"
#include "search.h"
#include "transpositiontable.h"

std::string STARTPOS = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

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
    else return;

    Position::set(fen);

    while (cmd >> token)
        Position::commit_move(uci_to_move(token));
}

void moves(std::istringstream& ss) {

    std::string token;

    while (ss >> token)
        Position::commit_move(uci_to_move(token));
}

void go(std::istringstream& ss) {

    std::string token;

    ss >> token;

    if (token == "nodes")
    {
        int depth;
        ss >> depth;
        Search::count_nodes(depth);
    } 
    else if (token == "movetime")
    {
        uint64_t thinktime;
        ss >> thinktime;
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
            Search::clear();
            TranspositionTable::clear();
            RepetitionTable::clear();
        }

        else if (token == "position") position(ss);
        else if (token == "go")       go(ss);
        else if (token == "moves")    moves(ss);
        else if (token == "perft")    Debug::perft(ss);
        else if (token == "debug")    Debug::go();
        else if (token == "gameinfo") Debug::gameinfo();
        
    } while (cmd != "quit");
}

Move uci_to_move(const std::string& uci) {

    if (Position::white_to_move())
    {
        MoveList<WHITE> moves;

        for (Move m : moves)
            if (move_to_uci(m) == uci)
                return m;
    }
    else
    {
        MoveList<BLACK> moves;

        for (Move m : moves)
            if (move_to_uci(m) == uci)
                return m;
    }

    return NO_MOVE;
}
