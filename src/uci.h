
#ifndef UCI_H
#define UCI_H

#include "types.h"

#include <string>

namespace UCI {

  inline const char* START_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
  inline constexpr uint64_t thinktime = 2500;

  void loop();
  void handle_uci();
  void handle_d();
  void handle_debug();
  void handle_newgame();
  void handle_isready();
  void handle_position(std::string input);
  void handle_go(std::string input);
  void handle_gameloop(std::string input);

  void move_prompt(std::string move);
  std::string int_to_UCI(bool white, Move move);
  int UCI_to_int(bool white, std::string uci_move);
  bool starts_with(const std::string& str, const std::string& prefix);
  void trim(std::string& str);
  void make_ai_move();

}

#endif
