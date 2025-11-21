// Copyright 2025 Liam Hopkins
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <iostream>
#include <sstream>
#include <vector>

namespace tetris {

using std::vector;

const int SCREEN_WIDTH = 600;
const int SCREEN_HEIGHT = 480;

const int BLOCK_SIZE = 20;
const int TIME_THRESHOLD = 200;

typedef vector<vector<int>> Shape;

std::ostream& operator<<(std::ostream& os, const vector<int>& vec) {
    std::stringstream res;
    res << vec.front();
    for (auto it = vec.begin() + 1; it != vec.end(); ++it)
        res << ", " << *it;

    return os << "<" << res.str() << ">";
}

Shape transpose(const Shape& mat) {
    int n = mat.size();
    int m = mat[0].size();
    Shape res(m, vector<int>(n));

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            res[j][i] = mat[i][j];
        }
    }

    return res;
}

Shape rotateLeft(const Shape& mat) {
    Shape res = transpose(mat);
    int n = res.size();

    for (int i = 0; i < (n / 2); ++i) {
        std::swap(res[i], res[n - i - 1]);
    }

    return res;
}

Shape rotateRight(const Shape& mat) {
    Shape res = transpose(mat);
    int m = res[0].size();

    for (int i = 0; i < (m / 2); ++i) {
        for (int j = 0; j < m+1; ++j) {
            std::swap(res[j][i], res[j][m - i - 1]);
        }
    }

    return res;
}

class Block {
 public:
    Block(SDL_Color color_, Shape shape_)
        : color{color_}, shape{shape_} {
            w = shape_[0].size();
        }

    void Draw(SDL_Renderer*, const SDL_Point&);
    void Rotate();

 private:
    SDL_Color color;
    Shape shape;
    int w;
    int rotation = 0;
};

void Block::Draw(SDL_Renderer* renderer, const SDL_Point& dst) {
    auto shape_ = rotateRight(shape);
    int y = 0;
    for (auto row : shape_) {
        int x = 0;
        for (auto col : row) {
            if (col == 0)
                continue;
            SDL_Rect square = {
                dst.x + x * BLOCK_SIZE,
                dst.y + y * BLOCK_SIZE,
                BLOCK_SIZE,
                BLOCK_SIZE
            };
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
            SDL_RenderFillRect(renderer, &square);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
            SDL_RenderDrawRect(renderer, &square);

            ++x;
        }
        ++y;
    }
}

void Block::Rotate() {
    if (w == 2)
        return;
    ++rotation;

    if (w == 4 && rotation > 1)
        rotation = 0;

    if (rotation > 3)
        rotation = 0;
}

class SquareBlock : public Block {
 public:
    SquareBlock() : Block({0xFF, 0xFF, 0, 0xFF}, {{1, 1}, {1, 1}}) {}
};


class IBlock : public Block {
 public:
    IBlock() : Block({0, 0xFF, 0xFF, 0xFF}, {{1, 1, 1, 1}}) {}
};

class SBlock : public Block {
 public:
    SBlock() : Block({0xFF, 0, 0, 0xFF}, {{1, 1, 0}, {0, 1, 1}}) {}
};

class ZBlock : public Block {
 public:
    ZBlock() : Block({0xFF, 0, 0, 0xFF}, {{0, 1, 1}, {1, 1, 0}}) {}
};

class LBlock : public Block {
 public:
    LBlock() : Block({0, 0xFF, 0, 0xFF}, {{1, 1, 1}, {0, 0, 1}}) {}
};

class JBlock : public Block {
 public:
    JBlock() : Block({0, 0xFF, 0, 0xFF}, {{1, 1, 1}, {1, 0, 0}}) {}
};

class TBlock : public Block {
 public:
    TBlock() : Block({0, 0, 0xFF, 0xFF}, {{1, 1, 1}, {0, 1, 0}}) {}
};

}  // namespace tetris

int main() {
    SDL_Window* win = NULL;
    SDL_Renderer* renderer = NULL;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }

    win = SDL_CreateWindow(
            "", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, tetris::SCREEN_WIDTH, tetris::SCREEN_HEIGHT,
            SDL_WINDOW_SHOWN);
    if (win == NULL) {
        std::cout << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }

    renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        std::cout << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }

    const tetris::Block blocks[] = {tetris::LBlock{}, tetris::IBlock{}, tetris::SBlock{}, tetris::TBlock{},
      tetris::SquareBlock{}, tetris::JBlock{}, tetris::ZBlock{}};


    SDL_Rect playfield = {200, 20, 12 * tetris::BLOCK_SIZE, 22 * tetris::BLOCK_SIZE};

    // tetris::Block moveBlock = blocks[rand() % 7];
    // tetris::Block nextBlock = blocks[rand() % 7];
    tetris::Block moveBlock = blocks[0];
    tetris::Block nextBlock = blocks[1];

    SDL_Point moveBlockLocation = {tetris::SCREEN_WIDTH / 2, playfield.y + 200};
    SDL_Point nextBlockLocation = {tetris::SCREEN_WIDTH - 100, 18 * tetris::BLOCK_SIZE};

    // auto currentTicks = SDL_GetTicks();
    // auto lastMoveTicks = currentTicks;

    SDL_Event e;
    bool quit = false;
    int idx = 1;
    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            switch (e.type) {
                case SDL_QUIT:
                    quit = true;
                    break;
                case SDL_KEYDOWN:
                    switch (e.key.keysym.sym) {
                        case 'q':
                            quit = true;
                            break;
                        case 'z':
                            moveBlock = nextBlock;
                            idx++;
                            if (idx > 4)
                                idx = 0;
                            nextBlock = blocks[idx];
                            break;
                        case 's':
                            moveBlock.Rotate();
                            break;
                    }
                    break;
            }
        }

        // currentTicks = SDL_GetTicks();

        // Non-white background
        SDL_SetRenderDrawColor(renderer, 0xBA, 0xBA, 0xBA, 0xFF);
        SDL_RenderClear(renderer);

        /*
        if (currentTicks - lastMoveTicks > tetris::TIME_THRESHOLD) {
          moveBlockLocation.y = moveBlockLocation.y + tetris::BLOCK_SIZE;
          if (moveBlockLocation.y >= 21 * tetris::BLOCK_SIZE) {
            moveBlock = nextBlock;
            moveBlockLocation = {tetris::SCREEN_WIDTH / 2, playfield.y};
            nextBlock = blocks[rand() % 7];
          }
          lastMoveTicks = currentTicks;
        }
        */

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
        SDL_RenderDrawRect(renderer, &playfield);

        moveBlock.Draw(renderer, moveBlockLocation);
        nextBlock.Draw(renderer, nextBlockLocation);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);

    SDL_Quit();

    return 0;
}
