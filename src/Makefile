
HFILES = bitboard.h debug.h evaluation.h movegen.h movelist.h moveordering.h position.h search.h transpositiontable.h types.h uci.h

FLAGS = -fpermissive -std=c++17 -march=native -w -O3 -c

OBJS = bitboard.o debug.o main.o position.o search.o transpositiontable.o uci.o

chess: $(OBJS)
	g++ $(OBJS) -o chess

bitboard.o: bitboard.cpp bitboard.h types.h
	g++ $(FLAGS) bitboard.cpp

debug.o: debug.cpp $(HFILES)
	g++ $(FLAGS) debug.cpp

main.o: main.cpp bitboard.h search.h position.h uci.h types.h transpositiontable.h
	g++ $(FLAGS) main.cpp

position.o: position.cpp bitboard.h position.h transpositiontable.h uci.h types.h
	g++ $(FLAGS) position.cpp

search.o: search.cpp $(HFILES)
	g++ $(FLAGS) search.cpp

transpositiontable.o: transpositiontable.cpp transpositiontable.h position.h types.h bitboard.h
	g++ $(FLAGS) transpositiontable.cpp

uci.o: uci.cpp $(HFILES)
	g++ $(FLAGS) uci.cpp

clean:
	rm -rf *.o *~ chess

