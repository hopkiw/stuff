CXXFLAGS=-Wall -Wextra -Werror -std=c++17 -O0
default: run

run: tetris
	./tetris

tetris: tetris.cpp
	g++ ${CXXFLAGS} -g $^ -o $@ -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer
