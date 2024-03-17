
#include "mouse.h"
#include "gamestate.h"
#include "defs.h"
#include "types.h"

#include <thread>
#include <windows.h>
#include <iostream>

void click(int square) {
  SetCursorPos(pixel_board[square][0], pixel_board[square][1]);
  INPUT input[2];
  ZeroMemory(input, sizeof(input));

  input[0].type = INPUT_MOUSE;
  input[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

  input[1].type = INPUT_MOUSE;
  input[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;

  SendInput(2, input, sizeof(INPUT));
}

void Mouse::make_move(Move move) {
  int from = move & 0x3f;
  int to = (move >> 6) & 0x3f;
  int movetype = type_of(move);
                
  if (!GameState::white_to_move) {
    from ^= 63;
    to ^= 63;
  }
                
  switch (movetype) {
  case NORMAL:
  case ENPASSANT:
    click(from);
    click(to);
    return;
  case PROMOTION:
    click(from);
    click(to);
    click(to);
    return;
  case SHORTCASTLE:
    if (GameState::white_to_move) {
      click(3);
      click(1);
    }
    else {
      click(4);
      click(6);
    }
    return;
  case LONGCASTLE:
    if (GameState::white_to_move) {
      click(3);
      click(5);
    }
    else {
      click(4);
      click(2);
    }
    return;
  }
}

void Mouse::test() {
  int move = 51 + (35 << 6);
  make_move(move);
}
