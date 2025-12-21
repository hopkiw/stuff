CXXFLAGS=-Wall -Wextra -Werror -std=c++17 -O0
default: run

run: boxes
	./boxes

boxes: boxes.cpp
	g++ ${CXXFLAGS} -g $^ -o $@ -lSDL2
