
#include "mouse.h"
#include "gamestate.h"
#include "defs.h"
#include <thread>
#include <windows.h>
#include <iostream>

//clang++ -std=c++20 -lUser32 mouse.h mouse.cpp main.cpp

namespace Chess {

namespace Mouse {

	void make_move(int move) {
		int from = move & lsb_6;
		int to = (move >> 6) & lsb_6;
		int movetype = move >> 12;
		
		if (!GameState::white_to_move) {
			from ^= 63;
			to ^= 63;
		}
		
		switch (movetype) {
		case STANDARD:
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

	void click(int square) {
		SetCursorPos(pixel_board[square][0],pixel_board[square][1]);
		INPUT input[2];
		ZeroMemory(input, sizeof(input));

		input[0].type = INPUT_MOUSE;
		input[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

		input[1].type = INPUT_MOUSE;
		input[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;

		SendInput(2, input, sizeof(INPUT));
	}

	void test() {
		int move = 51 + (35 << 6);
		make_move(move);
	}

}

}
