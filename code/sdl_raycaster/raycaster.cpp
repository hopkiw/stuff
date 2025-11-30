// Copyright 2025 Liam Hopkins
#include "include/raycaster.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
// #include <SDL2/SDL_ttf.h>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>


namespace raycaster {

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

SDL_Window* win = NULL;
SDL_Renderer* renderer = NULL;
// TTF_Font* font = NULL;

bool Init() {
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

    /*
    font = TTF_OpenFont("assets/TerminusTTF-4.46.0.ttf", 20);
    if (font == NULL) {
        std::cout << "Font could not be created. TTF Error: " << TTF_GetError() << std::endl;
        return false;
    }
    */

    return true;
}

}  // namespace raycaster

typedef struct Point {
    int x;
    int y;
} Point;

typedef struct FPoint {
    float x;
    float y;
} FPoint;

std::ostream& operator<<(std::ostream& os, const Point& p) {
    return os << "{" << p.x << "," << p.y << "}";
}

std::ostream& operator<<(std::ostream& os, const FPoint& p) {
    return os << "{" << p.x << "," << p.y << "}";
}

int main() {
    if (!raycaster::Init())
        return 1;

    std::vector<std::vector<int>> map = {
        {0, 1, 0, 0, 0, 0, 0, 0},
        {0, 1, 0, 0, 0, 0, 1, 0},
        {0, 1, 0, 0, 0, 1, 1, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 1, 0, 0},
    };
    // std::vector<std::vector<int>> map(4, std::vector<int>(8, 0));
    const SDL_Color bgColor{0xBA, 0xBA, 0xBA, 0xFF};

    Point grid = {20, 20};  // rename gridpos or gridoffset or gridloc
    // Point clickDest = {128, 59};  // upper left
    Point clickDest = {520, 262};  // upper right
    FPoint rayStart = {2, 4};  // use real pos, not cell
    FPoint realrayStart = {220, 420};
    int WIDTH = 100;

    bool quit = false;
    while (!quit) {
        SDL_Event e;
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
                    }
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    if (e.button.button == SDL_BUTTON_LEFT) {
                        clickDest = {
                            e.button.x,
                            e.button.y,
                        };
                    }
                    if (e.button.button == SDL_BUTTON_RIGHT) {
                        std::cout << "clicked at " << e.button.x << "," << e.button.y << std::endl;
                    }
                    break;
                case SDL_MOUSEMOTION:
                    if (e.motion.state & SDL_BUTTON_LMASK) {
                        clickDest.x = e.motion.x;
                        clickDest.y = e.motion.y;
                    }
                    if (e.button.button == SDL_BUTTON_RIGHT) {
                        std::cout << "clicked at " << e.button.x << "," << e.button.y << std::endl;
                    }
                    break;
            }
        }

        SDL_SetRenderDrawColor(raycaster::renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
        SDL_RenderClear(raycaster::renderer);

        // draw top-down grid
        int x = 0, y = 0;
        for (const auto& row : map) {
            for (auto i : row) {
                SDL_Rect r = {grid.x + x * WIDTH, grid.y + y * WIDTH, WIDTH, WIDTH};
                switch (i) {
                    case 0:
                        SDL_SetRenderDrawColor(raycaster::renderer, 0xFF, 0xFF, 0xFF, 0xFF);
                        SDL_RenderFillRect(raycaster::renderer, &r);
                        SDL_SetRenderDrawColor(raycaster::renderer, 0x0, 0x0, 0x0, 0xFF);
                        SDL_RenderDrawRect(raycaster::renderer, &r);
                        break;
                    case 1:
                        SDL_SetRenderDrawColor(raycaster::renderer, 0x0, 0x0, 0x0, 0xFF);
                        SDL_RenderFillRect(raycaster::renderer, &r);
                        SDL_SetRenderDrawColor(raycaster::renderer, 0xFF, 0xFF, 0xFF, 0xFF);
                        SDL_RenderDrawRect(raycaster::renderer, &r);
                        break;
                    case 2:
                        SDL_SetRenderDrawColor(raycaster::renderer, 0x0, 0x0, 0xFF, 0xFF);
                        SDL_RenderFillRect(raycaster::renderer, &r);
                        SDL_SetRenderDrawColor(raycaster::renderer, 0xFF, 0xFF, 0xFF, 0xFF);
                        SDL_RenderDrawRect(raycaster::renderer, &r);
                        break;
                }
                ++x;
            }
            x = 0;
            ++y;
        }

        Point step = {0, 0};
        Point mapCheck = {static_cast<int>(rayStart.x), static_cast<int>(rayStart.y)};

        FPoint rayDir = {
            clickDest.x - ((rayStart.x * WIDTH) + grid.x),
            clickDest.y - ((rayStart.y * WIDTH) + grid.x),
        };

        if (clickDest.x == rayStart.x * WIDTH + grid.x) {
            continue;
        }

        if (clickDest.y == rayStart.y * WIDTH + grid.y) {
            continue;
        }

        FPoint stepSize = {
            sqrt(1 + (rayDir.y / rayDir.x) * (rayDir.y / rayDir.x)),
            sqrt(1 + (rayDir.x / rayDir.y) * (rayDir.x / rayDir.y)),
        };

        FPoint rayLength;

        if (rayDir.x < 0) {
            step.x = -1;
            rayLength.x = (rayStart.x - mapCheck.x) * stepSize.x;
        } else {
            step.x = 1;
            rayLength.x = (mapCheck.x + 1 - rayStart.x) * stepSize.x;
        }

        if (rayDir.y < 0) {
            step.y = -1;
            rayLength.y = (rayStart.y - mapCheck.y) * stepSize.y;
        } else {
            step.y = 1;
            rayLength.y = (mapCheck.y + 1 - rayStart.y) * stepSize.y;
        }

        float fullwidth = (clickDest.x - realrayStart.x) / WIDTH;
        float fullheight = (clickDest.y - realrayStart.y) / WIDTH;
        float angle = atan(fullheight / fullwidth);

        FPoint intersection;
        float distance;
        while (true) {
            if (rayLength.x < rayLength.y) {
                // move horizontally
                mapCheck.x += step.x;
                distance = rayLength.x;
                rayLength.x += stepSize.x;

                float width = mapCheck.x - rayStart.x;
                if (step.x < 0)
                    width += 1;
                intersection.x = grid.x + (rayStart.x * WIDTH) + (width * WIDTH);
                float height = tan(angle) * width;
                intersection.y = grid.y + (rayStart.y * WIDTH) + (height * WIDTH);

            } else {
                // move vertically
                mapCheck.y += step.y;
                distance = rayLength.y;
                rayLength.y += stepSize.y;

                float height = mapCheck.y - rayStart.y;
                if (step.y < 0)
                    height += 1;
                intersection.y = grid.y + (rayStart.y * WIDTH) + (height * WIDTH);
                float width = height / tan(angle);
                intersection.x = grid.x + (rayStart.x * WIDTH) + (width * WIDTH);
            }

            if (mapCheck.y < 0 || mapCheck.x < 0) {
                break;
            }

            if (mapCheck.y > 4 || mapCheck.x > 7) {
                break;
            }

            if (map[mapCheck.y][mapCheck.x] == 1) {
                break;
            }

            if (distance > 8) {
                break;
            }
        }

        // draw the line
        SDL_SetRenderDrawColor(raycaster::renderer, 0xFF, 0x0, 0x0, 0xFF);
        SDL_RenderDrawLine(raycaster::renderer,
                realrayStart.x, realrayStart.y,
                intersection.x, intersection.y);

        SDL_RenderPresent(raycaster::renderer);
    }

    SDL_DestroyRenderer(raycaster::renderer);
    SDL_DestroyWindow(raycaster::win);

    SDL_Quit();
    return 0;
}
