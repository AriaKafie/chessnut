
#include "uci.h"
#include "defs.h"
#include "util.h"
#include "ui.h"
#include "position.h"


#include "transpositiontable.h"
#include "search.h"
#include "perft.h"
#include "bench.h"
#include "debug.h"
#include "mouse.h"
#include <algorithm>
#include <iomanip>
#include <cctype>
#include <iostream>

namespace UCI {

void loop() {

  Position::set(START_FEN);

  Position::them = WHITE;
  Position::us   = BLACK;
  std::string command = "";

  while (command != "quit") {

    std::getline(std::cin, command);
    trim(command);

    if (command == "uci") handle_uci();
    else if (command == "d") handle_d();
    else if (command == "debug") handle_debug();
    else if (command == "isready") handle_isready();
    else if (command == "ucinewgame") handle_newgame();
    else if (starts_with(command, "position")) handle_position(command);
    else if (starts_with(command, "go")) handle_go(command);
    else if (starts_with(command, "gameloop")) handle_gameloop(command);

  }

}

void handle_gameloop(std::string input) {

  UI::print_board();
  std::string command;

  if (input.find("white") != std::string::npos) {

    Position::us   = WHITE;
    Position::them = BLACK;

    if (Position::white_to_move()) {
      make_ai_move();
      std::cin >> command;
      while (command != "quit") {
        move_prompt(command);
        UI::print_board();
        make_ai_move();
        std::cin >> command;
      }
      return;
    }
    else {
      std::cin >> command;
      while (command != "quit") {
        move_prompt(command);
        UI::print_board();
        make_ai_move();
        std::cin >> command;
      }
      return;
    }
  }
  else {

    Position::us   = BLACK;
    Position::them = WHITE;

    if (Position::white_to_move()) {
      std::cin >> command;
      while (command != "quit") {
        move_prompt(command);
        UI::print_board();
        make_ai_move();
        std::cin >> command;
      }
      return;
    }
    else {
      make_ai_move();
      std::cin >> command;
      while (command != "quit") {
        move_prompt(command);
        UI::print_board();
        make_ai_move();
        std::cin >> command;
      }
      return;
    }
  }

}

void make_ai_move() {

  Move best_move = Position::white_to_move()
    ? Search::probe_white(thinktime)
    : Search::probe_black(thinktime);

  std::string sanstr = move_to_SAN(best_move);
  Mouse::make_move(best_move);
  Position::do_commit(best_move);
  UI::print_board();
  std::cout << Util::get_fen() << "\n";
  std::cout << sanstr << "\ndepth searched: " << std::dec << Debug::last_depth_searched << "\n";

}

void move_prompt(std::string move) {

  std::string input = move;
  int to_int = UI::movestring_to_int(input);

  while (to_int == -1) {
    std::cout << "invalid\n";
    std::cin >> input;
    to_int = UI::movestring_to_int(input);
  }

  Position::do_commit(to_int);

}

void handle_newgame() {

  TranspositionTable::clear();

}

void handle_debug() {
  std::cout << std::uppercase << std::left;
  std::cout << std::setw(9) << "tt:" << (TranspositionTable::disabled ? "disabled\n" : "enabled\n");
  std::cout << std::setw(9) << "endgame:" << (Position::endgame() ? "true\n" : "false\n");
  std::cout << std::setw(9) << "mopup:" << (Position::mopup() ? "true\n" : "false\n");
  std::cout << std::setw(9) << "last depth searched:" << std::dec << Debug::last_depth_searched << "\n";
}

void handle_go(std::string input) {

  if (input.find("perft") != std::string::npos) {
    std::cout << std::dec << "";
    int depth = std::stoi(input.substr(9));
    Perft::go(depth);
  }
  else if (input.find("nodes") != std::string::npos) {
    std::cout << std::dec << "";
    int depth = std::stoi(input.substr(9));
    Bench::count_nodes(depth);
  }
  else if (input.find("debug") != std::string::npos) {
    Debug::go();
  }
  else {
    Position::us   =  Position::side_to_move;
    Position::them = !Position::side_to_move;
    int bestmove = Position::white_to_move() ? Search::probe_white(thinktime) : Search::probe_black(thinktime);
    std::cout << "bestmove " << move_to_UCI(bestmove) << "\n";
  }

}

void handle_uci () {

  std::cout << "uciok\n";

}

void handle_isready () {

  std::cout << "readyok\n";

}

void handle_position (std::string input) {

  input = input.substr(9);
  if (starts_with(input, "startpos")) {
    Position::set(START_FEN);
  }
  else {
    if (input.find("moves") == std::string::npos) {
      Position::set(input.substr(4));
      return;
    }
    else {
      size_t fen_end = input.find("moves") - 1;
      std::string fen = input.substr(4, fen_end);
      Position::set(fen);
    }
  }
  size_t moves_start = input.find("moves");
  if (moves_start != std::string::npos) {
    input = input.substr(input.find("moves") + 6);
    while (input.find(' ') != std::string::npos) {
      std::string uci_move = input.substr(0, input.find(' '));
      int computer_move = UCI_to_move(Position::white_to_move(), uci_move);
      Position::do_commit(computer_move);
      input = input.substr(input.find(' ') + 1);
    }
    int computer_move = UCI_to_move(Position::white_to_move(), input);
    Position::do_commit(computer_move);
  }

}

std::string move_to_UCI(Move m) {
  return UI::coords[from_sq(m)] + UI::coords[to_sq(m)] + (type_of(m) == PROMOTION ? "q" : "");
}

Move UCI_to_move(bool white_to_move, std::string uci_move) {

  if (uci_move.length() < 4 || uci_move.length() > 5) {
    std::cout << "invalid move in UCI_to_move: " << uci_move;
    std::exit(0);
  }

  int promotion_append = 0;
  int enpassant_append = 0;

  if (white_to_move) {
    if (piece_on(E1) == W_KING) {
      if (uci_move == "e1g1") return W_SCASTLE;
      if (uci_move == "e1c1") return W_LCASTLE;
    }

    if (uci_move.length() == 5) {
      if (uci_move.find('q') == 4) {
        promotion_append = PROMOTION;
      }
      else {
        std::cout << "invalid move in UCI_to_move: " << uci_move;
        std::exit(0);
      }
    }

    int from = 0;
    int to = 0;

    for (int square = 0; square < 64; square++) {
      if (UI::coords[square] == uci_move.substr(0, 2))
        from = square;
      if (UI::coords[square] == uci_move.substr(2, 2))
        to = square;
    }

    int computer_move = from + (to << 6);

    if ((board[from] == W_PAWN) && ((std::abs(to - from) % 2) != 0) && (board[to] == NO_PIECE))
      enpassant_append = ENPASSANT;

    return computer_move + enpassant_append + promotion_append;
  }
  else {
    if (piece_on(E8) == B_KING) {
      if (uci_move == "e8g8") return B_SCASTLE;
      if (uci_move == "e8c8") return B_LCASTLE;
    }

    if (uci_move.length() == 5) {
      if (uci_move.find('q') == 4) {
        promotion_append = PROMOTION;
      }
      else {
        std::cout << "invalid move in UCI_to_move: " << uci_move;
        std::exit(0);
      }
    }

    int from = 0;
    int to = 0;

    for (int square = 0; square < 64; square++) {
      if (UI::coords[square] == uci_move.substr(0, 2))
        from = square;
      if (UI::coords[square] == uci_move.substr(2, 2))
        to = square;
    }

    int computer_move = from + (to << 6);

    if ((board[from] == B_PAWN) && (std::abs(to - from) & 1) && (board[to] == NO_PIECE))
      enpassant_append = ENPASSANT;

    return computer_move + enpassant_append + promotion_append;
  }

}

void handle_d() {

  Position::them = WHITE;
  Position::us   = BLACK;
  UI::print_board();
  std::cout << "Fen: " << Util::get_fen() << "\n";
  std::cout << "Key: " << std::hex << std::uppercase << Position::key() << "\n\n";

}

bool starts_with(const std::string& str, const std::string& prefix) {

  return str.compare(0, prefix.length(), prefix) == 0;

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

}
