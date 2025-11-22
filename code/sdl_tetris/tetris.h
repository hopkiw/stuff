// Copyright 2025 Liam Hopkins
#ifndef CODE_SDL_TETRIS_TETRIS_H_
#define CODE_SDL_TETRIS_TETRIS_H_

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <iostream>
#include <string>
#include <vector>

namespace tetris {

typedef std::vector<std::vector<int>> Shape;

class Block {
 public:
    Block(const std::string& name_, SDL_Color color_, Shape shape_) :
      name{name_},
      color{color_},
      orig{shape_},
      shape{shape_} { }

    Shape AddToLines(SDL_Point, const Shape&) const;
    void Draw(SDL_Renderer*, const SDL_Point&) const;
    void DrawText() const;
    bool GetCollision(SDL_Point p, const Shape&) const;
    bool GetCollisionX(SDL_Point p, const Shape&) const;
    std::string GetName() const { return name; }
    Shape GetShape() const { return shape; }
    void Rotate();

 private:
    SDL_Point getOffset() const;

    std::string name;
    SDL_Color color;
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
