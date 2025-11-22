// Copyright 2025 Liam Hopkins
#include "tetris.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>

using std::vector;

std::ostream& operator<<(std::ostream& os, const vector<int>& vec) {
    std::stringstream res;
    res << vec.front();
    for (auto it = vec.begin() + 1; it != vec.end(); ++it)
        res << ", " << *it;

    return os << "<" << res.str() << ">";
}

std::ostream& operator<<(std::ostream& os, const tetris::Block& block) {
    return os << "Block<" << block.GetName() << ">";
}

std::ostream& operator<<(std::ostream& os, const SDL_Point& p) {
    return os << "SDL_Point{" << p.x << "," << p.y << "}";
}

namespace tetris {

// TODO(hopkiw): refactor so all game logic uses COLxROW
// TODO(hopkiw): remove SDL references

const int SCREEN_WIDTH = 600;
const int SCREEN_HEIGHT = 480;

const int BLOCK_SIZE = 20;
const int TIME_THRESHOLD = 200;

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

    for (size_t i = 0; i < res.size(); ++i) {
        for (int j = 0; j < m / 2; ++j) {
            std::swap(res[i][j], res[i][m - j - 1]);
        }
    }

    return res;
}


SDL_Point Block::getOffset() const {
    int w = orig[0].size();
    int xoffset, yoffset;
    switch (rotation) {
        case 0:
            xoffset = (w == 4) ? -2 : -1;
            yoffset = 0;
            break;
        case 1:
            xoffset = (w == 4) ? 0 : -1;
            yoffset = (w == 4) ? -2 : -1;
            break;
        case 2:
            xoffset = -1;
            yoffset = -1;
            break;
        case 3:
            xoffset = 0;
            yoffset = -1;
            break;
    }
    return SDL_Point{xoffset, yoffset};
}

void Block::Draw(SDL_Renderer* renderer, const SDL_Point& dst) const {
    auto offset = getOffset();
    int y = 0;
    for (const auto& row : shape) {
        int x = 0;
        for (auto col : row) {
            if (col != 0) {
                SDL_Rect square = {
                    (dst.x + offset.x * BLOCK_SIZE) + (x * BLOCK_SIZE),
                    (dst.y + offset.y * BLOCK_SIZE) + (y * BLOCK_SIZE),
                    BLOCK_SIZE,
                    BLOCK_SIZE
                };

                SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
                SDL_RenderFillRect(renderer, &square);
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
                SDL_RenderDrawRect(renderer, &square);
            }
            ++x;
        }
        ++y;
    }
}

void Block::DrawText() const {
    for (const auto& row : shape)
        std::cout << row << std::endl;
}

Shape Block::AddToLines(SDL_Point p, const Shape& lines) const {
    auto offset = getOffset();
    auto copy = lines;
    size_t shaperows = shape.size(), shapecols = shape[0].size();
    for (size_t shaperow = 0; shaperow < shaperows; ++shaperow) {
        for (size_t shapecol = 0; shapecol < shapecols; ++shapecol) {
            if (shape[shaperow][shapecol])
                copy[shaperow + p.y + offset.y][shapecol + p.x + offset.x] = 1;
        }
    }
    return copy;
}

bool Block::GetCollision(SDL_Point p, const Shape& lines) const {
    size_t shaperows = shape.size(), shapecols = shape[0].size();
    auto offset = getOffset();

    for (size_t row = 0; row < shaperows; ++row) {
        for (size_t col = 0; col < shapecols; ++col) {
            if (shape[row][col] == 0)
                continue;

            if (static_cast<int>(p.x + offset.x + col) < 0)
                return true;

            if ((p.x + offset.x + col) >= lines[0].size())
                return true;

            if ((p.y + offset.y + row) >= lines.size())
                return true;

            if (lines[p.y + offset.y + row][p.x + offset.x + col] == 1)
                return true;
        }
    }

    return false;
}

void Block::Rotate() {
    // hardcoded skip square
    if (orig[0].size() == 2)
        return;

    ++rotation;

    // hardcoded limit IBlock
    if (orig[0].size() == 4 && rotation > 1)
        rotation = 0;

    // limit rotations
    if (rotation > 3)
        rotation = 0;

    // rotate!
    auto copy = orig;
    for (int i = 0; i < rotation; ++i)
      copy = rotateRight(copy);
    shape = copy;
}

}  // namespace tetris

int clearFilledLines(tetris::Shape* lines) {
  int deleted = 0;
  for (auto it = lines->begin(); it != lines->end();) {
    int blocks = std::count_if(it->begin(), it->end(), [](auto a) { return a == 1; });

    if (static_cast<size_t>(blocks) == it->size()) {
      it = lines->erase(it);
      ++deleted;
    } else {
      ++it;
    }
  }
  for (int i = 0; i < deleted; ++i)
    lines->insert(lines->begin(), std::vector<int>(10, 0));

  return deleted;
}

int main() {
    srand(time(NULL));

    SDL_Window* win = NULL;
    SDL_Renderer* renderer = NULL;
    TTF_Font* font = NULL;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    win = SDL_CreateWindow(
            "", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, tetris::SCREEN_WIDTH, tetris::SCREEN_HEIGHT,
            SDL_WINDOW_SHOWN);
    if (win == NULL) {
        std::cout << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        std::cout << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    if (TTF_Init() == -1) {
      std::cout << "SDL_ttf could not initialize. TTF Error: " << TTF_GetError() << std::endl;
      return 1;
    }

    font = TTF_OpenFont("/usr/share/fonts/truetype/terminus/TerminusTTF-4.46.0.ttf", 20);
    if (font == NULL) {
        std::cout << "Font could not be created. TTF Error: " << TTF_GetError() << std::endl;
        return 1;
    }

    const tetris::Block blocks[7] = {
        tetris::LBlock,
        tetris::IBlock,
        tetris::SBlock,
        tetris::TBlock,
        tetris::Square,
        tetris::JBlock,
        tetris::ZBlock
    };

    SDL_Rect playfield = {200, 20, 10 * tetris::BLOCK_SIZE, 20 * tetris::BLOCK_SIZE};
    tetris::Shape lines(20, std::vector<int>(10, 0));

    tetris::Block moveBlock = blocks[rand() % 7];
    tetris::Block nextBlock = blocks[rand() % 7];

    SDL_Point spawnBlockLocation = {5, 0};
    SDL_Point moveBlockLocation = spawnBlockLocation;
    SDL_Point nextBlockLocation = {tetris::SCREEN_WIDTH - 100, 18 * tetris::BLOCK_SIZE};

    auto currentTicks = SDL_GetTicks();
    auto lastMoveTicks = currentTicks;

    int score = 0;

    SDL_Event e;
    bool quit = false;
    bool gameOver = false;
    bool paused = false;
    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            switch (e.type) {
                case SDL_QUIT:
                    quit = true;
                    break;
                case SDL_KEYDOWN:
                    switch (e.key.keysym.sym) {
                        case 'q':
                            if (!gameOver)
                                gameOver = true;
                            else
                                quit = true;
                            break;
                        case SDLK_SPACE:
                            paused = !paused;
                            break;
                        case 'j':
                        case 'k':
                            moveBlock.Rotate();
                            break;
                        case 'h':
                            if (!moveBlock.GetCollision(
                                        {moveBlockLocation.x - 1, moveBlockLocation.y},
                                        lines))
                                --moveBlockLocation.x;
                            break;
                        case 'l':
                            if (!moveBlock.GetCollision(
                                        {moveBlockLocation.x + 1, moveBlockLocation.y},
                                        lines))
                                ++moveBlockLocation.x;
                            break;
                    }
                    break;
            }
        }

        if (!gameOver && !paused) {
            currentTicks = SDL_GetTicks();

            if (currentTicks - lastMoveTicks > tetris::TIME_THRESHOLD) {
                  if (moveBlock.GetCollision(
                        {moveBlockLocation.x, moveBlockLocation.y + 1}, lines)) {
                      if (moveBlockLocation.y < 2)
                          gameOver = true;

                      lines = moveBlock.AddToLines(moveBlockLocation, lines);
                      score += clearFilledLines(&lines);

                      moveBlock = nextBlock;
                      moveBlockLocation = spawnBlockLocation;
                      nextBlock = blocks[rand() % 7];
                  } else {
                      ++moveBlockLocation.y;
                  }
                  lastMoveTicks = currentTicks;
            }
        }

        // Non-white background
        SDL_SetRenderDrawColor(renderer, 0xBA, 0xBA, 0xBA, 0xFF);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderFillRect(renderer, &playfield);

        if (!gameOver) {
          for (int row = 0; row < 20; ++row) {
              for (int col = 0; col < 10; ++col) {
                  SDL_Rect r = {
                      playfield.x + col * tetris::BLOCK_SIZE,
                      playfield.y + row * tetris::BLOCK_SIZE,
                      tetris::BLOCK_SIZE,
                      tetris::BLOCK_SIZE
                  };

                  if (lines[row][col] == 0 || paused) {
                      SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
                  } else {
                      SDL_SetRenderDrawColor(renderer, 0xA9, 0xA9, 0xA9, 0xFF);
                      SDL_RenderFillRect(renderer, &r);
                      SDL_SetRenderDrawColor(renderer, 0xA9, 0xA9, 0xA9, 0xFF);
                      SDL_RenderDrawRect(renderer, &r);
                  }
              }
          }
        } else {
            // gray playfield
            SDL_SetRenderDrawColor(renderer, 0xA9, 0xA9, 0xA9, 0xFF);
            SDL_RenderFillRect(renderer, &playfield);

            // game over message
            SDL_Surface* textSurface = TTF_RenderText_Solid(font, "Game over", {0, 0, 0, 0xFF});
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            SDL_Rect r = {
                playfield.x + (5 * tetris::BLOCK_SIZE) - (textSurface->w / 2),
                playfield.y + (10 * tetris::BLOCK_SIZE) - textSurface->h,
                textSurface->w,
                textSurface->h
            };

            SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
            SDL_RenderFillRect(renderer, &r);
            SDL_RenderCopy(renderer, textTexture, NULL, &r);
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
        SDL_RenderDrawRect(renderer, &playfield);

        if (!gameOver && !paused) {
            SDL_Point realMoveBlockLocation = {
                playfield.x + moveBlockLocation.x * tetris::BLOCK_SIZE,
                playfield.y + moveBlockLocation.y * tetris::BLOCK_SIZE
            };
            moveBlock.Draw(renderer, realMoveBlockLocation);
            nextBlock.Draw(renderer, nextBlockLocation);

            SDL_Surface* textSurface = TTF_RenderText_Solid(font, "Next block:", {0, 0, 0, 0xFF});
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            SDL_Rect r = {nextBlockLocation.x - 50, nextBlockLocation.y - 50, textSurface->w, textSurface->h};
            SDL_RenderCopy(renderer, textTexture, NULL, &r);
        }

        {
          SDL_Surface* tSurface = TTF_RenderText_Solid(font, "Lines :", {0, 0, 0, 0xFF});
          SDL_Texture* tTexture = SDL_CreateTextureFromSurface(renderer, tSurface);
          SDL_Rect r = {nextBlockLocation.x - 50, nextBlockLocation.y - 110, tSurface->w, tSurface->h};
          SDL_RenderCopy(renderer, tTexture, NULL, &r);

          r.x = r.x + tSurface->w + 5;
          tSurface = TTF_RenderText_Solid(font, std::to_string(score).c_str(), {0, 0, 0, 0xFF});
          tTexture = SDL_CreateTextureFromSurface(renderer, tSurface);
          r.w = tSurface->w;
          r.h = tSurface->h;
          SDL_RenderCopy(renderer, tTexture, NULL, &r);
        }

        if (paused) {
            // pause message
            SDL_Surface* textSurface = TTF_RenderText_Solid(font, "Paused", {0, 0, 0, 0xFF});
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            SDL_Rect r = {
                playfield.x + (5 * tetris::BLOCK_SIZE) - (textSurface->w / 2),
                playfield.y + (10 * tetris::BLOCK_SIZE) - textSurface->h,
                textSurface->w,
                textSurface->h
            };

            SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
            SDL_RenderFillRect(renderer, &r);
            SDL_RenderCopy(renderer, textTexture, NULL, &r);
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);

    SDL_Quit();

    return 0;
}
