
HEADERS = bitboard.h debug.h evaluation.h movegen.h movelist.h moveordering.h position.h search.h transpositiontable.h types.h uci.h

FLAGS = -fpermissive -std=c++17 -march=native -w -O3 -c

OBJS = bitboard.o debug.o main.o position.o search.o transpositiontable.o uci.o

chess: $(OBJS)
	g++ $(OBJS) -o chess

bitboard.o: bitboard.cpp $(HEADERS)
	g++ $(FLAGS) bitboard.cpp

debug.o: debug.cpp $(HEADERS)
	g++ $(FLAGS) debug.cpp

main.o: main.cpp $(HEADERS)
	g++ $(FLAGS) main.cpp

position.o: position.cpp $(HEADERS)
	g++ $(FLAGS) position.cpp

search.o: search.cpp $(HEADERS)
	g++ $(FLAGS) search.cpp

transpositiontable.o: transpositiontable.cpp $(HEADERS)
	g++ $(FLAGS) transpositiontable.cpp

uci.o: uci.cpp $(HEADERS)
	g++ $(FLAGS) uci.cpp

clean:
	rm -rf *.o *~ chess

debug:
	g++ -fpermissive -std=c++17 -march=native -w -g -odebug *.cpp
