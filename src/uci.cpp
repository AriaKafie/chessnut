
#include "uci.h"

#include <iostream>
#include <sstream>

#include "debug.h"
#include "movegen.h"
#include "position.h"
#include "search.h"
#include "transpositiontable.h"

void position(std::istringstream& is)
{
    std::string fen, token;
    is >> token;

    if (token == "startpos")
    {
        fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -";
        is >> token;
    }

    else if (token == "fen")
        for (;is >> token && token != "moves"; fen += token + " ");

    Position::set(fen);
    
    for (;is >> token && uci_to_move(token); Position::commit_move(uci_to_move(token)));
}

void go(std::istringstream& is)
{
    if (is.eof()) { Search::go(); return; }

    std::string token;
    is       >> token;

    if (int movetime; token == "movetime" && is >> movetime)
        Search::go(movetime);

    else if (int depth; token == "nodes" && is >> depth)
        Search::count_nodes(depth);

    else
    {
        is.str(is.str()), is.clear();

        int wtime = 100, winc = 0, btime = 100, binc = 0;

        for (std::string token; is >> token;)
        {
            if (token == "wtime") is >> wtime;
            if (token == "winc")  is >> winc;
            if (token == "btime") is >> btime;
            if (token == "binc")  is >> binc;
        }

        Search::go(Position::white_to_move() ? (wtime + winc) / 15 : (btime + binc) / 15);
    }
}

void UCI::loop()
{
    Position::set("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -");

    std::string cmd, token;

    do
    {
        std::getline(std::cin, cmd);

        std::istringstream is(cmd);
        is >> token;

        if      (token == "uci")     std::cout << "id name chestnut\n"
                                               << "id author Aria Kafie\n"
                                               << "uciok"               << std::endl;
        else if (token == "isready") std::cout << "readyok"             << std::endl;
        else if (token == "d")       std::cout << Position::to_string() << std::endl;
        else if (token == "fen")     std::cout << Position::fen()       << std::endl;

        else if (token == "ucinewgame")
        {
            Search::clear();
            TranspositionTable::clear();
            RepetitionTable::clear();
        }

        else if (token == "position") position(is);
        else if (token == "go")       go(is);
        else if (token == "moves")    for (;is >> token && uci_to_move(token); Position::commit_move(uci_to_move(token)));
        else if (token == "perft")    Debug::perft(is);
        else if (token == "debug")    Debug::go();
        
    } while (cmd != "quit");
}

Move uci_to_move(const std::string& uci)
{
    for (Move list[MAX_MOVES], *m = list, *end = get_moves(list); m != end; m++)
        if (move_to_uci(*m) == uci) return *m;

    return NULLMOVE;
}
