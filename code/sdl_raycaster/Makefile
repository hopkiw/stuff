CXXFLAGS=-Wall -Wextra -Werror -std=c++17 -O0
default: run

run: raycaster
	./raycaster

example: example.cpp
	g++ ${CXXFLAGS} -g $^ -o $@ -lSDL2

raycaster: raycaster.cpp
	g++ ${CXXFLAGS} -g $^ -o $@ -lSDL2
