CXXFLAGS=-Wall -Wextra -Werror -std=c++17 -O3
default: run

run: tetris
	./tetris

tetris: main.cpp
	g++ ${CXXFLAGS} -g main.cpp -o $@ -lSDL2 -lSDL2_image -lSDL2_ttf
