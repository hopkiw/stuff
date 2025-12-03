CXXFLAGS=-Wall -Wextra -Werror -std=c++17 -O0
default: run

run: starfield
	./starfield

starfield: starfield.o
	g++ ${CXXFLAGS} -g $^ -o $@ -lSDL2 -lSDL2_ttf

starfield.o: starfield.cpp
	g++ ${CXXFLAGS} -g -c $^ -o $@
