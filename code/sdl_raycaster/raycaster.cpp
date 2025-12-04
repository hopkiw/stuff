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

float getRayLength(FPoint rayStart, FPoint rayDir, const std::vector<std::vector<int>>& map) {
    FPoint stepSize = {
        sqrt(1 + (rayDir.y / rayDir.x) * (rayDir.y / rayDir.x)),
        sqrt(1 + (rayDir.x / rayDir.y) * (rayDir.x / rayDir.y)),
    };

    Point mapCheck = {
        static_cast<int>(rayStart.x),
        static_cast<int>(rayStart.y)
    };

    FPoint rayStartFract = {
        rayStart.x - mapCheck.x,
        rayStart.y - mapCheck.y,
    };

    FPoint rayLength;
    Point step = {1, 1};
    if (rayDir.x < 0) {
        step.x = -1;
        rayLength.x = rayStartFract.x * stepSize.x;
    } else {
        rayLength.x = (1 - rayStartFract.x) * stepSize.x;
    }

    if (rayDir.y < 0) {
        step.y = -1;
        rayLength.y = rayStartFract.y * stepSize.y;
    } else {
        rayLength.y = (1 - rayStartFract.y) * stepSize.y;
    }

    float distance = 0;
    while (true) {
        if (rayLength.x < rayLength.y) {
            // move horizontally
            mapCheck.x += step.x;
            distance = rayLength.x;  // ordering matters
            rayLength.x += stepSize.x;
        } else {
            // move vertically
            mapCheck.y += step.y;
            distance = rayLength.y;  // ordering matters
            rayLength.y += stepSize.y;
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
    }
    return distance;
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
    Point raycast = {500, 20};  // rename gridpos or gridoffset or gridloc
    Point clickDest = {63, 188};  // upper left
    FPoint rayStart = {3.38, 2.42};

    int TILESIZE = 50;

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
                        std::cout << "dragged to " << e.button.x << "," << e.button.y << std::endl;
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
                SDL_Rect r = {grid.x + (x * TILESIZE), grid.y + (y * TILESIZE), TILESIZE, TILESIZE};
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

        if (clickDest.x == grid.x + (rayStart.x * TILESIZE)) {
            SDL_RenderPresent(raycaster::renderer);
            continue;
        }

        if (clickDest.y == grid.y + (rayStart.y * TILESIZE)) {
            SDL_RenderPresent(raycaster::renderer);
            continue;
        }

        FPoint realDest = {
            static_cast<float>(clickDest.x - grid.x) / TILESIZE,
            static_cast<float>(clickDest.y - grid.y) / TILESIZE,
        };

        float fullwidth = realDest.x - rayStart.x;
        float fullheight = realDest.y - rayStart.y;
        float angle = atan(fullheight / fullwidth);

        Point step = {1, 1};
        if (fullwidth < 0)
            step.x = -1;
        if (fullheight < 0)
            step.y = -1;

        FPoint rayDir = {
            abs(cos(angle)) * step.x,
            abs(sin(angle)) * step.y,
        };

        float distance = getRayLength(rayStart, rayDir, map);


        // draw raycast window
        SDL_Rect raycastrect = {raycast.x, raycast.y, 640, 480};
        SDL_SetRenderDrawColor(raycaster::renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderFillRect(raycaster::renderer, &raycastrect);

        for (float i = 320, innerangle = angle; i < 340; ++i, innerangle += .007) {
            Point istep = {1, 1};
            if (fullwidth < 0)
                istep.x = -1;
            if (fullheight < 0)
                istep.y = -1;

            FPoint irayDir = {
                abs(cos(innerangle)) * istep.x,
                abs(sin(innerangle)) * istep.y,
            };

            float idistance = getRayLength(rayStart, irayDir, map);

            FPoint iintersection = {
                rayStart.x + (irayDir.x * idistance),
                rayStart.y + (irayDir.y * idistance),
            };

            // draw the line
            SDL_SetRenderDrawColor(raycaster::renderer, 0xFF, 0x0, 0x0, 0xFF);
            SDL_RenderDrawLine(raycaster::renderer,
                    grid.x + (rayStart.x * TILESIZE),
                    grid.y + (rayStart.y * TILESIZE),
                    grid.x + (iintersection.x * TILESIZE),
                    grid.y + (iintersection.y * TILESIZE));

            int irayHeight = idistance * 100;
            SDL_SetRenderDrawColor(raycaster::renderer, 0, 0xFF, 0, 0xFF);
            SDL_RenderDrawLine(raycaster::renderer,
                    raycast.x + i, raycast.y + 480,
                    raycast.x + i, raycast.y + irayHeight);
        }

        // Draw intersection point
        FPoint intersection = {
            rayStart.x + (rayDir.x * distance),
            rayStart.y + (rayDir.y * distance),
        };
        SDL_Rect r = {
            grid.x + static_cast<int>(intersection.x * TILESIZE),
            grid.y + static_cast<int>(intersection.y * TILESIZE),
            8,
            8};
        SDL_SetRenderDrawColor(raycaster::renderer, 0x0, 0xFF, 0x0, 0xFF);
        SDL_RenderFillRect(raycaster::renderer, &r);
        SDL_SetRenderDrawColor(raycaster::renderer, 0x0, 0x0, 0x0, 0xFF);
        SDL_RenderDrawRect(raycaster::renderer, &r);

        // draw the line
        SDL_SetRenderDrawColor(raycaster::renderer, 0xFF, 0x0, 0x0, 0xFF);
        SDL_RenderDrawLine(raycaster::renderer,
                grid.x + (rayStart.x * TILESIZE),
                grid.y + (rayStart.y * TILESIZE),
                grid.x + (intersection.x * TILESIZE),
                grid.y + (intersection.y * TILESIZE));

        SDL_RenderPresent(raycaster::renderer);
    }

    SDL_DestroyRenderer(raycaster::renderer);
    SDL_DestroyWindow(raycaster::win);

    SDL_Quit();
    return 0;
}
