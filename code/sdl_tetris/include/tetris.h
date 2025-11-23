// Copyright 2025 Liam Hopkins
#ifndef CODE_SDL_TETRIS_TETRIS_H_
#define CODE_SDL_TETRIS_TETRIS_H_

#include <iostream>
#include <string>
#include <vector>

namespace tetris {

typedef std::vector<std::vector<int>> Shape;

typedef struct Point {
    int x;
    int y;
} Point;

typedef struct Color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
} Color;

class Block {
 public:
     Block(const std::string& name_, Color color_, Shape shape_) :
         name{name_},
         color{color_},
         orig{shape_},
         shape{shape_} { }

    Shape AddToLines(Point, const Shape&) const;
    // void Draw(SDL_Renderer*, const SDL_Point&) const;
    Color GetColor() const { return color; }
    Point GetOffset() const;
    Shape GetShape() const { return shape; }
    std::string GetName() const { return name; }

    bool GetCollision(Point p, const Shape&) const;
    void DrawText() const;
    void Rotate();

 private:
    std::string name;
    Color color;
    Shape orig;
    Shape shape;
    int rotation = 0;
};

const Block IBlock("IBlock", {0x00, 0xFF, 0xFF, 0xFF}, {{1, 1, 1, 1}});
const Block Square("Square", {0xFF, 0x00, 0x00, 0xFF}, {{1, 1},    {1, 1}});
const Block ZBlock("ZBlock", {0xFF, 0x00, 0x00, 0xFF}, {{1, 1, 0}, {0, 1, 1}});
const Block SBlock("SBlock", {0x80, 0x00, 0x80, 0xFF}, {{0, 1, 1}, {1, 1, 0}});
const Block LBlock("LBlock", {0x00, 0xFF, 0x00, 0xFF}, {{1, 1, 1}, {0, 0, 1}});
const Block JBlock("JBlock", {0x00, 0x00, 0xFF, 0xFF}, {{1, 1, 1}, {1, 0, 0}});
const Block TBlock("TBlock", {0xFF, 0xFF, 0x00, 0xFF}, {{1, 1, 1}, {0, 1, 0}});

}  // namespace tetris

#endif  // CODE_SDL_TETRIS_TETRIS_H_
