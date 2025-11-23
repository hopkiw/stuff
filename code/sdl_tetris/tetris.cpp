// Copyright 2025 Liam Hopkins
#include "include/tetris.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

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

Point Block::GetOffset() const {
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
    return Point{xoffset, yoffset};
}

Shape Block::AddToLines(Point p, const Shape& lines, int color) const {
    std::cout << "adding a shape with color " << color << std::endl;
    auto offset = GetOffset();
    auto copy = lines;
    size_t shaperows = shape.size(), shapecols = shape[0].size();
    for (size_t shaperow = 0; shaperow < shaperows; ++shaperow) {
        for (size_t shapecol = 0; shapecol < shapecols; ++shapecol) {
            if (shape[shaperow][shapecol]) {
                int rowidx = shaperow + p.y + offset.y;
                int colidx = shapecol + p.x + offset.x;
                if (rowidx < 0) {
                    std::cout << "skip blocks above screen" << std::endl;
                    continue;  // don't draw off screen!
                }

                copy[rowidx][colidx] = color;
            }
        }
    }
    return copy;
}

bool Block::GetCollision(Point p, const Shape& lines) const {
    size_t shaperows = shape.size(), shapecols = shape[0].size();
    auto offset = GetOffset();

    for (size_t row = 0; row < shaperows; ++row) {
        for (size_t col = 0; col < shapecols; ++col) {
            if (shape[row][col] == 0)
                continue;

            if (static_cast<int>(p.x + offset.x + col) < 0)
                return true;

            if ((p.x + offset.x + col) >= lines[0].size())
                return true;

            if (static_cast<int>(p.y + offset.y + row) < 0)
                continue;  // ignore blocks off screen vertically

            if ((p.y + offset.y + row) >= lines.size())
                return true;

            if (lines[p.y + offset.y + row][p.x + offset.x + col] != -1)
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

const SDL_Color ColorCyan   {0x00, 0xFF, 0xFF, 0xFF};
const SDL_Color ColorRed    {0xFF, 0x00, 0x00, 0xFF};
const SDL_Color ColorPurple {0x80, 0x00, 0x80, 0xFF};
const SDL_Color ColorGreen  {0x00, 0xFF, 0x00, 0xFF};
const SDL_Color ColorBlue   {0x00, 0x00, 0xFF, 0xFF};
const SDL_Color ColorYellow {0xFF, 0xFF, 0x00, 0xFF};


void DrawBlock(SDL_Renderer* renderer, const Block& block, const SDL_Color color, const Point& dst) {
    auto offset = block.GetOffset();
    auto shape = block.GetShape();

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


void Label::Draw(SDL_Renderer* renderer, TTF_Font* font) {
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, text.c_str(), {0, 0, 0, 0xFF});
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_Rect r = {
        p.x,
        p.y,
        textSurface->w,
        textSurface->h
    };
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderCopy(renderer, textTexture, NULL, &r);
}

int clearFilledLines(Shape* lines) {
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

SDLTetris::SDLTetris() :
    playfield{200, 20, 10 * BLOCK_SIZE, 20 * BLOCK_SIZE},
    centerRect{
        playfield.x + (5 * BLOCK_SIZE) - (30),
        playfield.y + (10 * BLOCK_SIZE) - (20),
        90,
        20
    },
    lines{Shape(20, std::vector<int>(10, -1))},
    colors{
        ColorGreen,
        ColorCyan,
        ColorRed,
        ColorYellow,
        ColorRed,
        ColorBlue,
        ColorPurple,
    },
    spawnBlockLocation{5, 0},
    moveBlockLocation{spawnBlockLocation},
    nextBlockLocation{SCREEN_WIDTH - 100, 18 * BLOCK_SIZE} {
}

bool SDLTetris::Init() {
    if (init)
        return true;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cout << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }

    win = SDL_CreateWindow(
        "", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (win == NULL) {
        std::cout << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }

    renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        std::cout << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }

    if (TTF_Init() == -1) {
        std::cout << "SDL_ttf could not initialize. TTF Error: " << TTF_GetError() << std::endl;
        return false;
    }

    font = TTF_OpenFont("assets/TerminusTTF-4.46.0.ttf", 20);
    if (font == NULL) {
        std::cout << "Font could not be created. TTF Error: " << TTF_GetError() << std::endl;
        return false;
    }

    if (!(Mix_Init(MIX_INIT_MP3) & MIX_INIT_MP3)) {
        std::cout << "SDL_mixer doesn't support MP3" << Mix_GetError() << std::endl;
        return false;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cout << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << std::endl;
        return false;
    }

    // '02 A-Type Music (version 1.1).mp3'  '03 B-Type Music.mp3'  '04 C-Type Music.mp3'
    music = Mix_LoadMUS("assets/03 B-Type Music.mp3");
    if (music == NULL) {
        std::cout << "Failed to load beat music! SDL_mixer Error: " <<  Mix_GetError() << std::endl;;
        return false;
    }

    return init = true;
}

bool SDLTetris::Rotate(Block* block) {
    auto copy = *block;
    copy.Rotate();
    if (copy.GetCollision(moveBlockLocation, lines))
        return false;

    block->Rotate();
    return true;
}

bool SDLTetris::MoveDown(const Block& block) {
    if (block.GetCollision({moveBlockLocation.x, moveBlockLocation.y + 1}, lines))
        return false;

    ++moveBlockLocation.y;
    return true;
}

bool SDLTetris::MoveLeft(const Block& block) {
    if (block.GetCollision({moveBlockLocation.x - 1, moveBlockLocation.y}, lines))
        return false;

    --moveBlockLocation.x;
    return true;
}

bool SDLTetris::MoveRight(const Block& block) {
    if (block.GetCollision({moveBlockLocation.x + 1, moveBlockLocation.y}, lines))
        return false;

    ++moveBlockLocation.x;
    return true;
}

void SDLTetris::handleEvents(Block& block) {
    SDL_Event e;
        while (SDL_PollEvent(&e) != 0) {
            switch (e.type) {
                case SDL_QUIT:
                    quit = true;
                    break;
                case SDL_KEYDOWN:
                    switch (e.key.keysym.sym) {
                        case 'q':
                            if (!gameOver) {
                                gameOver = true;
                                paused = false;
                            } else {
                                quit = true;
                            }
                            break;
                        case SDLK_SPACE:
                            if (!gameOver)
                                paused = !paused;
                            break;
                        case 'k':
                            Rotate(&block);
                            break;
                        case 'j':
                            MoveLeft(block);
                            break;
                        case 'l':
                            MoveRight(block);
                            break;
                        case 'n':
                            drop = true;
                            break;
                    }
                    break;
            }
        }
}

void SDLTetris::Run() {
    if (!init)
        return;

    srand(time(NULL));

    std::vector<colorBlock> blocks = {
        colorBlock{LBlock, 0},
        colorBlock{IBlock, 1},
        colorBlock{SBlock, 2},
        colorBlock{TBlock, 3},
        colorBlock{Square, 2},
        colorBlock{JBlock, 4},
        colorBlock{ZBlock, 5},
    };
    colorBlock moveBlock = blocks[rand() % blocks.size()];
    colorBlock nextBlock = blocks[rand() % blocks.size()];

    std::vector<Label> labels = {
        {"njkl to move", {20, 20}},
        {"spacebar to pause", {20, 40}},
        {"q to quit", {20, 60}},
    };
    Label gameovermessage = {"Game over", {centerRect.x - 20, centerRect.y}};
    Label pausedmessage = {"Paused", {
            playfield.x + (5 * BLOCK_SIZE) - (30),
            playfield.y + (10 * BLOCK_SIZE) - (20),
        }};

    auto currentTicks = SDL_GetTicks();
    auto lastMoveTicks = currentTicks;

    while (!quit) {
        handleEvents(moveBlock.block);

        /*
        bool musicplaying = Mix_PlayingMusic() == 1;
        bool musicpaused = Mix_PausedMusic() == 1;

        if (gameOver) {
            if (musicplaying)
                Mix_HaltMusic();
        } else {
            // game is not over
            if (paused && !musicpaused)
                Mix_PauseMusic();

            if (!paused && musicpaused)
                Mix_ResumeMusic();

            if (!paused && !musicplaying) {
                Mix_PlayMusic(music, -1);
                Mix_VolumeMusic(0x20);
            }
        }
        */

        // Game logic
        if (!gameOver && !paused) {
            currentTicks = SDL_GetTicks();

            if (currentTicks - lastMoveTicks > TIME_THRESHOLD || drop) {
                do {
                    bool did = MoveDown(moveBlock.block);
                    if (did) {
                        lastMoveTicks = currentTicks;
                    } else {
                        std::cout << moveBlock.block << " collision" << std::endl;
                        drop = false;
                        if (moveBlockLocation.y < 2)
                            gameOver = true;

                        auto newlines = moveBlock.block.AddToLines(moveBlockLocation, lines, moveBlock.color);
                        if (newlines == lines)
                            std::cout << "wtf" << std::endl;
                        lines = newlines;
                        score += clearFilledLines(&lines);

                        moveBlock = nextBlock;
                        moveBlockLocation = spawnBlockLocation;
                        nextBlock = blocks[rand() % blocks.size()];
                    }
                } while (drop);
            }
        }

        // Background
        SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
        SDL_RenderClear(renderer);

        // Playfield
        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderFillRect(renderer, &playfield);

        // Help message
        for (auto label : labels)
            label.Draw(renderer, font);

        // Tiles
        if (!gameOver) {
            for (int row = 0; row < 20; ++row) {
                for (int col = 0; col < 10; ++col) {
                    SDL_Rect r = {
                        playfield.x + col * BLOCK_SIZE,
                        playfield.y + row * BLOCK_SIZE,
                        BLOCK_SIZE,
                        BLOCK_SIZE
                    };

                    if (lines[row][col] == -1 || paused) {
                        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
                    } else {
                        auto color = colors[lines[row][col]];
                        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
                        SDL_RenderFillRect(renderer, &r);
                        SDL_RenderDrawRect(renderer, &r);
                    }
                }
            }
        } else {
            // Game over
            SDL_SetRenderDrawColor(renderer, someColor.r, someColor.g, someColor.b, someColor.a);
            SDL_RenderFillRect(renderer, &playfield);

            SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
            SDL_Rect r = {
                centerRect.x - 20,
                centerRect.y,
                centerRect.w,
                centerRect.h,
            };
            SDL_RenderFillRect(renderer, &r);

            gameovermessage.Draw(renderer, font);
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
        SDL_RenderDrawRect(renderer, &playfield);

        if (!gameOver && !paused) {
            DrawBlock(renderer, moveBlock.block,
                colors[moveBlock.color],
                {
                    playfield.x + moveBlockLocation.x * BLOCK_SIZE,
                    playfield.y + moveBlockLocation.y * BLOCK_SIZE,
                });

            auto nextBlockColor = colors[nextBlock.color];
            DrawBlock(renderer, nextBlock.block,
                {nextBlockColor.r, nextBlockColor.g, nextBlockColor.b, nextBlockColor.a},
                nextBlockLocation);

            Label nextblockmessage = {"Next block:", {
                nextBlockLocation.x - 50,
                nextBlockLocation.y - 50,
            }};
            nextblockmessage.Draw(renderer, font);
        }

        Label linesmessage = {"Lines:", {
            nextBlockLocation.x - 50,
            nextBlockLocation.y - 110,
        }};
        linesmessage.Draw(renderer, font);

        Label linecount = {std::to_string(score), {
            nextBlockLocation.x + 20,
            nextBlockLocation.y - 110,
        }};
        linecount.Draw(renderer, font);

        if (paused)
            pausedmessage.Draw(renderer, font);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);

    SDL_Quit();
}

}  // namespace tetris

int main() {
    tetris::SDLTetris tetris;
    tetris.Init();
    tetris.Run();
    return 0;
}
