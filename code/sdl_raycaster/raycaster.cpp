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

    /*
    if (TTF_Init() == -1) {
        std::cout << "SDL_ttf could not initialize. TTF Error: " << TTF_GetError() << std::endl;
        return false;
    }

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

typedef struct DPoint {
    double x;
    double y;
} DPoint;

std::ostream& operator<<(std::ostream& os, const Point& p) {
    return os << "{" << p.x << "," << p.y << "}";
}

std::ostream& operator<<(std::ostream& os, const FPoint& p) {
    return os << "{" << p.x << "," << p.y << "}";
}

int main() {
    if (!raycaster::Init()) {
        std::cout << "genuinely, wtf" << std::endl;
        return 7;
    }

    std::vector<std::vector<int>> map = {
        {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0},
        {0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
    };
    // std::vector<std::vector<int>> map(4, std::vector<int>(8, 0));
    const SDL_Color bgColor{0xBA, 0xBA, 0xBA, 0xFF};

    Point grid = {20, 20};  // rename gridpos or gridoffset or gridloc
    Point raycast = {500, 20};  // rename gridpos or gridoffset or gridloc
    // Point clickDest = {63, 188};  // upper left
    // FPoint rayStart = {3.38, 2.42};
    FPoint rayStart = {2.2, 1.2};

    int TILESIZE = 50;

    bool quit = false;
    bool print = true;
    Uint32 time, oldTime;
    double planeX = 0.09313920531951236, planeY = -0.653395047756294;
    double dirX = -1, dirY = 0;
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

        // float fullwidth = realDest.x - rayStart.x;
        // float fullheight = realDest.y - rayStart.y;
        // float angle = atan(fullheight / fullwidth);
        // double planeX = 0, planeY = 0.66;
        // double posX = 22, posY = 12;  //x and y start position

        // draw raycast window
        SDL_Rect raycastrect = {raycast.x, raycast.y, 640, 480};
        SDL_SetRenderDrawColor(raycaster::renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderFillRect(raycaster::renderer, &raycastrect);


        for (int i = 0; i < 640; ++i) {
            /*
            FPoint rayDir = {
                abs(cos(angle)) * step.x,
                abs(sin(angle)) * step.y,
            };
            */
            // float innerangle = angle + (i * 0.008);
            double cameraX = 2 * i / 640.0 - 1;  // x-coordinate in camera space

            DPoint rayDir = {
                dirX + planeX * cameraX,
                dirY + planeY * cameraX,
            };

            Point mapCheck = {
                static_cast<int>(rayStart.x),
                static_cast<int>(rayStart.x),
            };


            FPoint rayStartFract = {
                rayStart.x - mapCheck.x,
                rayStart.y - mapCheck.y,
            };


            /*
            FPoint stepSize = {
                sqrt(1 + (rayDir.y / rayDir.x) * (rayDir.y / rayDir.x)),
                sqrt(1 + (rayDir.x / rayDir.y) * (rayDir.x / rayDir.y)),
            };
            */

            DPoint stepSize = {
                  (rayDir.x == 0) ? 1e30 : abs(1 / rayDir.x),
                  (rayDir.y == 0) ? 1e30 : abs(1 / rayDir.y),
            };

            Point step;
            FPoint rayLength;

            if (rayDir.x < 0) {
                step.x = -1;
                rayLength.x = rayStartFract.x * stepSize.x;
            } else {
                step.x = 1;
                rayLength.x = (1 - rayStartFract.x) * stepSize.x;
            }

            if (rayDir.y < 0) {
                step.y = -1;
                rayLength.y = rayStartFract.y * stepSize.y;
            } else {
                step.y = 1;
                rayLength.y = (1 - rayStartFract.y) * stepSize.y;
            }


            // float distance = 0;
            int side;
            float perpWallDist = 0;
            while (true) {
                if (rayLength.x < rayLength.y) {
                    // move horizontally
                    mapCheck.x += step.x;
                    // distance = rayLength.x;  // ordering matters
                    rayLength.x += stepSize.x;
                    side = 0;
                } else {
                    // move vertically
                    mapCheck.y += step.y;
                    // distance = rayLength.y;  // ordering matters
                    rayLength.y += stepSize.y;
                    side = 1;
                }

                if (side == 0)
                    perpWallDist = (rayLength.y - stepSize.x);
                else
                    perpWallDist = (rayLength.y - stepSize.y);

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

            // Draw intersection point
            /*
            DPoint intersection = {
                rayStart.x + (rayDir.x * distance),
                rayStart.y + (rayDir.y * distance),
            };
            SDL_Rect r = {
                grid.x + static_cast<int>(intersection.x * TILESIZE),
                grid.y + static_cast<int>(intersection.y * TILESIZE),
                6,
                6};
            SDL_SetRenderDrawColor(raycaster::renderer, 0x0, 0xFF, 0x0, 0xFF);
            SDL_RenderFillRect(raycaster::renderer, &r);
            SDL_SetRenderDrawColor(raycaster::renderer, 0x0, 0x0, 0x0, 0xFF);
            SDL_RenderDrawRect(raycaster::renderer, &r);
            */

            DPoint intersection = {
                rayStart.x + (rayDir.x * perpWallDist),
                rayStart.y + (rayDir.y * perpWallDist),
            };

            // draw the line
            SDL_SetRenderDrawColor(raycaster::renderer, 0xFF, 0x0, 0x0, 0xFF);
            SDL_RenderDrawLine(raycaster::renderer,
                    grid.x + (rayStart.x * TILESIZE),
                    grid.y + (rayStart.y * TILESIZE),
                    grid.x + (intersection.x * TILESIZE),
                    grid.y + (intersection.y * TILESIZE));

            int h = 480;
            int lineHeight = h / perpWallDist;
            int drawStart = ((0 - lineHeight) / 2) + (h / 2);

            if (drawStart < 0)
                drawStart = 0;

            int drawEnd = (lineHeight / 2) + (h / 2);
            if (drawEnd >= h)
                drawEnd = h - 1;

            if (print)
                std::cout << "drawing from " << drawStart << " to " << drawEnd << std::endl;

            if (drawStart > drawEnd)
                continue;

            if (drawEnd < 0)
                continue;

            SDL_SetRenderDrawColor(raycaster::renderer, 0, 0, 0, 0xFF);
            SDL_RenderDrawLine(raycaster::renderer,
                    raycast.x + i, raycast.y + drawStart,
                    raycast.x + i, raycast.y + drawEnd);
            /*
            SDL_Point center = {raycast.x + 320, raycast.y + 240};
            SDL_RenderDrawLine(raycaster::renderer,
                    center.x - 13 + i, center.y - ((240 / perpWallDist)),
                    center.x - 13 + i, center.y + ((240 / perpWallDist)));
            */
        }
        print = false;

        oldTime = time;
        time = SDL_GetTicks();
        double frameTime = (time - oldTime) / 1000.0;  // time this frame has taken, in seconds
        double rotSpeed = frameTime * 2.0;  // the constant value is in radians/second
        double oldDirX = dirX;
        dirX = dirX * cos(-rotSpeed) - dirY * sin(-rotSpeed);
        dirY = oldDirX * sin(-rotSpeed) + dirY * cos(-rotSpeed);
        double oldPlaneX = planeX;
        planeX = planeX * cos(-rotSpeed) - planeY * sin(-rotSpeed);
        planeY = oldPlaneX * sin(-rotSpeed) + planeY * cos(-rotSpeed);


        SDL_RenderPresent(raycaster::renderer);
    }

    SDL_DestroyRenderer(raycaster::renderer);
    SDL_DestroyWindow(raycaster::win);

    SDL_Quit();
    return 0;
}
