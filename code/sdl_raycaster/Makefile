CXXFLAGS=-Wall -Wextra -Werror -std=c++17 -O0
default: run

run: raycaster
	./raycaster

raycaster: raycaster.cpp
	g++ ${CXXFLAGS} -g $^ -o $@ -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer
