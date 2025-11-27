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
    Point clickDest = {457, 374};
    FPoint rayStart = {2, 4};  // use real pos, not cell
    int WIDTH = 100;

    bool quit = false;
    bool print = true;
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
                        print = true;
                    }
                    if (e.button.button == SDL_BUTTON_RIGHT) {
                        std::cout << "clicked at " << e.button.x << "," << e.button.y << std::endl;
                    }
                    break;
                case SDL_MOUSEMOTION:
                    if (e.motion.state & SDL_BUTTON_LMASK) {
                        clickDest.x = e.motion.x;
                        clickDest.y = e.motion.y;
                        print = true;
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
                    case 3:
                        SDL_SetRenderDrawColor(raycaster::renderer, 0x0, 0xFF, 0xFF, 0xFF);
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
            std::cout << "vertical line!" << std::endl;
            continue;
        }

        if (clickDest.y == rayStart.y * WIDTH + grid.y) {
            std::cout << "horizontal line!" << std::endl;
            continue;
        }

        FPoint stepSize = {
            sqrt(1 + (rayDir.y / rayDir.x) * (rayDir.y / rayDir.x)),
            sqrt(1 + (rayDir.x / rayDir.y) * (rayDir.x / rayDir.y)),
        };

        if (print) {
            std::cout << "mapCheck is " << mapCheck << std::endl;
            std::cout << "rayStart is " << rayStart << "(" << rayStart.x * WIDTH + grid.x << ","
                << rayStart.y * WIDTH + grid.y << ")" << std::endl;
            std::cout << "rayDir is " << rayDir << std::endl;
            std::cout << "stepSize is " << stepSize << std::endl;
            // print = false;
        }

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

        FPoint intersection;
        float distance;
        while (true) {
            if (rayLength.x < rayLength.y) {
                // move horizontally
                mapCheck.x += step.x;
                distance = rayLength.x;
                rayLength.x += stepSize.x;

                intersection.x = mapCheck.x * WIDTH + grid.x;
                int realY = grid.y + rayStart.y * WIDTH;
                float ratio = (static_cast<float>(rayDir.y) / static_cast<float>(rayDir.x));
                if (print) {
                    std::cout << "move horizontally (X)" << std::endl;
                    std::cout << "realY: " << realY << std::endl;
                    std::cout << "ratio: " << ratio << std::endl;
                }
                intersection.y = realY + (ratio * (mapCheck.x - rayStart.x) * WIDTH);
            } else {
                // move vertically
                mapCheck.y += step.y;
                distance = rayLength.y;
                rayLength.y += stepSize.y;

                intersection.y = (mapCheck.y + 1) * WIDTH + grid.y;
                float ratio = (static_cast<float>(rayDir.x) / static_cast<float>(rayDir.y));
                int realX = grid.x + rayStart.x * WIDTH;
                int realY = grid.y + rayStart.y * WIDTH;
                if (print) {
                    std::cout << "move vertically (Y)" << std::endl;
                    std::cout << "realY: " << realY << std::endl;
                    std::cout << "ratio: " << ratio << std::endl;
                }
                intersection.x = realX - (ratio * (realY - intersection.y));
            }

            if (print) {
                std::cout << "intersection at " << intersection << std::endl;
                std::cout << "distance is now " << distance << std::endl;
            }

            // finally draw the intersection
            SDL_Rect r = {static_cast<int>(intersection.x), static_cast<int>(intersection.y), 8, 8};
            SDL_SetRenderDrawColor(raycaster::renderer, 0x0, 0xFF, 0x0, 0xFF);
            SDL_RenderFillRect(raycaster::renderer, &r);

            SDL_SetRenderDrawColor(raycaster::renderer, 0xFF, 0x0, 0x0, 0xFF);
            SDL_RenderDrawLine(raycaster::renderer,
                    grid.x + rayStart.x * WIDTH, grid.y + rayStart.y * WIDTH,
                    intersection.x, intersection.y);


            if (mapCheck.y < 0 || mapCheck.x < 0) {
                if (print)
                    std::cout << "off map, found myself in block " << mapCheck << std::endl;

                print = false;
                break;
            }

            if (mapCheck.y > 4 || mapCheck.x > 7) {
                if (print)
                    std::cout << "off map, found myself in block " << mapCheck << std::endl;

                print = false;
                break;
            }

            if (distance > 8) {
                if (print)
                    std::cout << "too far, found myself in block " << mapCheck << std::endl;

                print = false;
                break;
            }

            if (map[mapCheck.y][mapCheck.x] == 1) {
                if (print)
                    std::cout << "collision in block " << mapCheck << std::endl;

                print = false;
                break;
            }
        }
        // quit = true;
        SDL_RenderPresent(raycaster::renderer);
    }

    SDL_DestroyRenderer(raycaster::renderer);
    SDL_DestroyWindow(raycaster::win);

    SDL_Quit();
    return 0;
}

// if angle is 45°, we imagine a triangle:
/*
 *
 *            b
 *           /|
 *          / |
 *      h  /  |
 *        /   |
 *       /    |
 *     a/_____|c
 *
 *
 *     h is fixed at 10
 *     a is the known source, angle a is known
 *     we need the location b in (x,y) coords
 *
 *     that's x = c - a, y = c - b (y increases descending)
 *
 *
 */
