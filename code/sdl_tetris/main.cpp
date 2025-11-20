// Copyright 2025 Liam Hopkins
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <iostream>

namespace tetris {

const int SCREEN_WIDTH = 600;
const int SCREEN_HEIGHT = 1080;

const int BLOCK_SIZE = 20;
const int TIME_THRESHOLD = 200;

class Block {
 public:
  Block(SDL_Color color_, uint8_t shapemask_, unsigned int w_)
    : color{color_}, shapemask{shapemask_}, w{w_} {}

  void Draw(SDL_Renderer*, const SDL_Point&);

 private:
  SDL_Color color;
  uint8_t shapemask;
  unsigned int w;
};

void Block::Draw(SDL_Renderer* renderer, const SDL_Point& dst) {
  uint8_t copy = shapemask;
  int x = 0, y = 0;
  SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
  while (copy) {
    if (copy & 1) {
      SDL_Rect square = {dst.x + x * BLOCK_SIZE, dst.y + y * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE};
      SDL_RenderFillRect(renderer, &square);
    }
    copy = copy >> 1;
    ++x;
    if (x % w == 0) {
      x = 0;
      ++y;
    }
  }
}

class SquareBlock : public Block {
 public:
  SquareBlock() : Block({0xFF, 0xFF, 0, 0xFF}, 0b1111, 2) {}
};

class IBlock : public Block {
 public:
  IBlock() : Block({0, 0xFF, 0xFF, 0xFF}, 0b1111, 1) {}
};

class SBlock : public Block {
 public:
  SBlock() : Block({0xFF, 0, 0, 0xFF}, 0b110011, 3) {}
};

class LBlock : public Block {
 public:
  LBlock() : Block({0, 0xFF, 0, 0xFF}, 0b110101, 2) {}
};

class TBlock : public Block {
 public:
  TBlock() : Block({0, 0, 0xFF, 0xFF}, 0b11101, 2) {}
};

bool init(SDL_Window** win, SDL_Renderer** renderer) {
  return true;
}

void close(SDL_Window** win, SDL_Renderer** renderer) {
}


}  // namespace tetris

int main() {
  SDL_Window* win = NULL;
  SDL_Renderer* renderer = NULL;

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cout << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
    return false;
  }

  win = SDL_CreateWindow(
      "", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, tetris::SCREEN_WIDTH, tetris::SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
  if (win == NULL) {
    std::cout << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
    return false;
  }

  renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (renderer == NULL) {
    std::cout << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
    return false;
  }

  const tetris::Block blocks[] = {tetris::LBlock{}, tetris::IBlock{}, tetris::SBlock{}, tetris::TBlock{}, tetris::SquareBlock{}};

  SDL_Event e;
  bool quit = false;
  int idx = 0;

  auto currentTicks = SDL_GetTicks();
  auto lastMoveTicks = currentTicks;

  tetris::Block moveBlock = blocks[rand() % 5];
  tetris::Block nextBlock = blocks[rand() % 5];

  SDL_Point moveBlockLocation = {tetris::SCREEN_WIDTH / 2, 0};
  SDL_Point nextBlockLocation = {tetris::SCREEN_WIDTH - 100, 18 * tetris::BLOCK_SIZE};

  SDL_Rect playfield = {200, 20, 12 * tetris::BLOCK_SIZE, 22 * tetris::BLOCK_SIZE};

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
              idx++;
              if (idx > 4)
                idx = 0;
              break;
          }
          break;
      }
    }

    currentTicks = SDL_GetTicks();

    // Non-white background
    SDL_SetRenderDrawColor(renderer, 0xBA, 0xBA, 0xBA, 0xFF);
    SDL_RenderClear(renderer);

    if (currentTicks - lastMoveTicks > tetris::TIME_THRESHOLD) {
      moveBlockLocation.y = moveBlockLocation.y + tetris::BLOCK_SIZE;
      if (moveBlockLocation.y >= 21 * tetris::BLOCK_SIZE) {
        moveBlock = nextBlock;
        moveBlockLocation = {tetris::SCREEN_WIDTH / 2, 0};
        nextBlock = blocks[rand() % 5];
      }
      lastMoveTicks = currentTicks;
    }

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
