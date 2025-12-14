CXXFLAGS=-Wall -Wextra -Werror -std=c++17 -O0
default: run

run: tracker
	./tracker

tracker: tracker.o
	g++ ${CXXFLAGS} -g $^ -o $@ -lSDL2 -lSDL2_ttf

tracker.o: tracker.cpp
	g++ ${CXXFLAGS} -g -c $^ -o $@
