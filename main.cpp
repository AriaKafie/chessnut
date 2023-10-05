
#include "Lookup.h"
#include "GameState.h"
#include "Zobrist.h"
#include "UCI.h"
/*
clang++ -march=native -lUser32 -std=c++20 -O3 Bench.h BlackCaptureGenerator.h BlackMoveGenerator.h Board.h Debug.h Defs.h Evaluation.h GameState.h Lookup.h Mouse.h MoveSorter.h Perft.h RepetitionTable.h search.h TranspositionTable.h UCI.h UI.h Util.h WhiteCaptureGenerator.h WhiteMoveGenerator.h Zobrist.h Bench.cpp BlackCaptureGenerator.cpp BlackMoveGenerator.cpp Board.cpp Debug.cpp Evaluation.cpp GameState.cpp Lookup.cpp main.cpp Mouse.cpp Perft.cpp RepetitionTable.cpp search.cpp TranspositionTable.cpp UCI.cpp UI.cpp WhiteCaptureGenerator.cpp WhiteMoveGenerator.cpp Zobrist.cpp
*/
using namespace Chess;

int main() {

	Lookup::init();
	GameState::init("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	Zobrist::init();
	UCI::loop();
	return 0;

}
