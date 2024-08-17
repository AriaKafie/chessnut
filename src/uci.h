
#ifndef UCI_H
#define UCI_H

#include "types.h"

#include <string>

constexpr auto STARTPOS = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

inline constexpr uint64_t think_time = 2500;

inline std::string square_to_uci(Square sq) { return std::string(1, "hgfedcba"[sq % 8]) + std::string(1, "12345678"[sq / 8]); }

inline Square uci_to_square(const std::string& uci) { return 8 * (uci[1] - '1') + 'h' - uci[0]; }

Move uci_to_move(const std::string& uci);

namespace UCI { void loop(); }

void handle_uci();
void handle_debug();
void handle_newgame();
void handle_isready();
void handle_gameloop(std::string input);

void move_prompt(std::string move);
std::string move_to_uci(Move m);
void trim(std::string& str);
void make_ai_move();

#endif
