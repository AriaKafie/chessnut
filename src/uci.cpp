
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
#include "perft.h"
#include "bench.h"
#include "debug.h"

void position(std::istringstream& cmd)
{
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
  } else
      return;

  Position::set(fen);

  while (cmd >> token)
    Position::do_commit(uci_to_move(token));
}

void moves(std::istringstream& cmd) {
  std::string token;
  while (cmd >> token)
  {
    Move move = uci_to_move(token);

    if (is_legal(move))
      Position::do_commit(move);
    else
    {
      std::cout << "invalid: " << token << "\n";
      return;
    }
  }
}

void go(std::istringstream& args) {

  int depth;
  std::string token;
  args >> token;

  if (token == "perft") {
    args >> depth;
    Perft::go(depth);
  } else if (token == "nodes") {
    args >> depth;
    Bench::count_nodes(depth);
  } else if (token == "debug") {
    Debug::go();
  } else if (token == "bench") {
    args >> depth;
    Perft::bench(depth);
  } else if (token == "movetime") {
    uint64_t thinktime;
    args >> thinktime;
    Search::go(thinktime);
  } else
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

    if (token == "uci")
      std::cout << "uciok\n";
    else if (token == "debug")
      handle_debug();
    else if (token == "isready")
      std::cout << "readyok\n";
    else if (token == "ucinewgame") {
      TranspositionTable::clear();
      RepetitionTable::clear();
    }
    else if (token == "position")
      position(ss);
    else if (token == "go")
      go(ss);
    else if (token == "moves")
      moves(ss);
    else if (token == "d")
      std::cout << Position::to_string();
    else if (token == "gameinfo")
      gameinfo();
    else if (token == "board")
      std::cout << brd() << "\n";

  } while (cmd != "quit");
}

Move uci_to_move(const std::string& uci) {

  Square from = uci_to_square(uci.substr(0, 2));
  Square to   = uci_to_square(uci.substr(2, 2));

  bool castling  = piece_type_on(from) == KING && std::regex_match(uci, std::regex("e[18][cg][18]"));
  bool promotion = uci.find('q') != std::string::npos;
  bool enpassant = piece_type_on(from) == PAWN && file_of(to) != file_of(from) && !piece_on(to);

  if (castling)
    return Position::white_to_move() ? file_of(to) & FILE_G ? W_SCASTLE : W_LCASTLE : file_of(to) & FILE_G ? B_SCASTLE : B_LCASTLE;

  return promotion ? make_move<PROMOTION>(from, to) : enpassant ? make_move<ENPASSANT>(from, to) : make_move(from, to);
}

void make_ai_move() {

  /*Move best_move = Search::bestmove(3000);

  std::string sanstr = move_to_SAN(best_move);
  Mouse::make_move(best_move);
  Position::do_commit(best_move);
  std::cout << Position::to_string();
  std::cout << sanstr << "\ndepth searched: " << std::dec << Debug::last_depth_searched << "\n";*/

}

void move_prompt(std::string move) {

  std::string input = move;
  int to_int = uci_to_move(input);

  while (to_int == -1) {
    std::cout << "invalid\n";
    std::cin >> input;
    to_int = uci_to_move(input);
  }

  Position::do_commit(to_int);

}

void handle_newgame() {

  TranspositionTable::clear();

}

void handle_debug() {
  std::cout << std::uppercase << std::left;
  std::cout << std::setw(9) << "endgame:" << (Position::endgame() ? "true\n" : "false\n");
  std::cout << std::setw(9) << "mopup:" << (Position::mopup() ? "true\n" : "false\n");
  std::cout << std::setw(9) << "last depth searched: " << std::dec << Debug::last_depth_searched << "\n";
}

void handle_uci () {

  std::cout << "uciok\n";

}

void handle_isready () {

  std::cout << "readyok\n";

}

std::string move_to_uci(Move m) {
  return square_to_uci(from_sq(m)) + square_to_uci(to_sq(m)) + (type_of(m) == PROMOTION ? "q" : "");
}

void trim(std::string& str) {

  size_t start = 0;
  size_t end = str.length();

  while (start < end && std::isspace(str[start])) {
    start++;
  }

  while (end > start && std::isspace(str[end - 1])) {
    end--;
  }

  str = str.substr(start, end - start);

}
