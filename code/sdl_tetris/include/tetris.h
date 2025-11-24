// Copyright 2025 Liam Hopkins
#ifndef TETRIS_H_
#define TETRIS_H_

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#include <iostream>
#include <string>
#include <vector>

namespace tetris {

const int SCREEN_WIDTH = 600;
const int SCREEN_HEIGHT = 480;

const int BLOCK_SIZE = 20;
const int TIME_THRESHOLD = 200;

const SDL_Color bgColor{0xBA, 0xBA, 0xBA, 0xFF};
const SDL_Color someColor{0xA9, 0xA9, 0xA9, 0xFF};

typedef std::vector<std::vector<int>> Shape;

typedef struct Point {
    int x;
    int y;
} Point;

class Block {
 public:
     Block() : name{"default"} { };
     Block(const std::string& name_, Shape shape_) :
         name{name_},
         orig{shape_},
         shape{shape_} { }

    Shape AddToLines(Point, const Shape&, int) const;
    Point GetOffset() const;
    Shape GetShape() const { return shape; }
    std::string GetName() const { return name; }

    bool GetCollision(Point, const Shape&) const;
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

class Label {
 public:
    Label() { std::cout << "Label<> default called" << std::endl; }
    Label(const std::string& text_, SDL_Point p_) : text{text_}, p{p_} {}
    void Draw(SDL_Renderer*, TTF_Font*);

 private:
    std::string text;
    SDL_Point p;
};

typedef struct colorBlock {
    Block block;
    int color;
} colorBlock;

typedef struct MoveBlock {
    colorBlock block;
    Point location;
} MoveBlock;

class SDLTetris {
 public:
    SDLTetris();

    bool Init();
    void Run();
    void Pause();
    void Quit();
    void Destroy();

    bool Rotate();
    bool MoveDown();
    bool MoveLeft();
    bool MoveRight();

    void DrawBlock(const Block&, const SDL_Color, const Point&);
    void Draw();

 private:
    void handleEvents();
    void gameLogic();

    SDL_Window* win = NULL;
    SDL_Renderer* renderer = NULL;
    TTF_Font* font = NULL;
    Mix_Music* music = NULL;

    SDL_Rect playfield;
    SDL_Rect centerRect;

    MoveBlock moveBlock;
    MoveBlock nextBlock;
    Shape lines;

    std::vector<SDL_Color> colors;
    std::vector<Label> labels;
    Label gameovermessage;
    Label pausedmessage;

    bool init = false;
    bool quit = false;
    bool gameOver = false;
    bool paused = false;
    bool drop = false;

    int score = 0;
};


}  // namespace tetris

#endif  // TETRIS_H_
