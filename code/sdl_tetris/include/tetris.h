// Copyright 2025 Liam Hopkins
#ifndef TETRIS_H_
#define TETRIS_H_

#include <iostream>
#include <string>
#include <vector>

namespace tetris {

typedef std::vector<std::vector<int>> Shape;

typedef struct Point {
    int x;
    int y;
} Point;

class Block {
 public:
     Block(const std::string& name_, Shape shape_) :
         name{name_},
         orig{shape_},
         shape{shape_} { }

    Shape AddToLines(Point, const Shape&, int) const;
    // void Draw(SDL_Renderer*, const SDL_Point&) const;
    Point GetOffset() const;
    Shape GetShape() const { return shape; }
    std::string GetName() const { return name; }

    bool GetCollision(Point p, const Shape&) const;
    void DrawText() const;
    void Rotate();

 private:
    std::string name;
    Shape orig;
    Shape shape;
    int rotation = 0;
};

const Block IBlock("IBlock", {{1, 1, 1, 1}});
const Block Square("Square", {{1, 1},    {1, 1}});
const Block ZBlock("ZBlock", {{1, 1, 0}, {0, 1, 1}});
const Block SBlock("SBlock", {{0, 1, 1}, {1, 1, 0}});
const Block LBlock("LBlock", {{1, 1, 1}, {0, 0, 1}});
const Block JBlock("JBlock", {{1, 1, 1}, {1, 0, 0}});
const Block TBlock("TBlock", {{1, 1, 1}, {0, 1, 0}});

}  // namespace tetris

#endif  // TETRIS_H_
