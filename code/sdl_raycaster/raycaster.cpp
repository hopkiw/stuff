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

typedef struct Camera {
    int x;
    int y;
    float angle;
} Camera;

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

int main() {
    if (!raycaster::Init())
        return 1;

    std::vector<std::vector<int>> map(4, std::vector<int>(8, 0));
    /*
    std::vector<std::vector<int>> map = {
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 1, 0, 1, 1, 1, 1, 0},
        {0, 1, 0, 1, 1, 1, 1, 0},
        {0, 1, 0, 1, 1, 1, 1, 0},
        {0, 1, 1, 1, 1, 1, 1, 0},
        {0, 1, 1, 1, 1, 0, 1, 0},
        {0, 1, 1, 1, 1, 1, 1, 0},
        {0, 0, 0, 0, 0, 0, 0, 0}};
    */

    float pi = 3.14159;
    const SDL_Color bgColor{0xBA, 0xBA, 0xBA, 0xFF};
    raycaster::Camera camera = {0, 0, 1*pi/4};

    struct {
        int x = 20;
        int y = 20;
    } grid;

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
                        case 'l':
                            camera.angle += 0.1f;
                            std::cout << "camera.angle: " << camera.angle<< std::endl;
                            break;
                        case 'h':
                            camera.angle -= 0.1f;
                            std::cout << "camera.angle: " << camera.angle<< std::endl;
                            break;
                    }
                    break;
            }
        }

        SDL_SetRenderDrawColor(raycaster::renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
        SDL_RenderClear(raycaster::renderer);

        int WIDTH = 100;
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

        // draw camera on top-down grid
        SDL_SetRenderDrawColor(raycaster::renderer, 0xFF, 0x0, 0x0, 0xFF);
        SDL_RenderDrawLine(raycaster::renderer, grid.x, grid.y, grid.x + 5 * WIDTH, grid.y + 2 * WIDTH);

        /*
        // camera base line
        int baseX = std::cos(camera.angle) * 5;
        int baseY = std::sin(camera.angle) * 5;
        SDL_RenderDrawLine(raycaster::renderer,
                grid.x + camera.x * WIDTH - baseX,
                grid.y + camera.y * WIDTH - baseY,
                grid.x + camera.x * WIDTH + baseX,
                grid.y + camera.y * WIDTH + baseY);

        // camera pointer line
        int lineY = std::cos(camera.angle) * 10;
        int lineX = std::sin(camera.angle) * 10;
        SDL_RenderDrawLine(raycaster::renderer,
                grid.x + camera.x * WIDTH,
                grid.y + camera.y * WIDTH,
                grid.x + camera.x * WIDTH + lineX,
                grid.y + camera.y * WIDTH - lineY);

                // soh cah toa sinθ = o/h  cosθ = a/h tanθ = o/a
                // 180° in any triangle. or π radians
                // 180 - 90 - camera.angle is 90 - camera.angle aka π - camera.angle
        */

        

        float ninety = pi / 2;
        std::cout << "ninety is: " << ninety << std::endl;
        float realRads = 0.38;
        std::cout << "realRads is: " << realRads << std::endl;
        std::cout << "positive angle tan is: " << tan(realRads) << std::endl;
        std::cout << "other angle tan is: " << tan(ninety - realRads) << std::endl;

        float xline = tan(realRads);
        float yline = tan(ninety - realRads);
        if (xline < yline) {
            std::cout << "first step along X axis" << std::endl;;
        } else {
            std::cout << "first step along Y axis" << std::endl;;
        }

        break;
        /*
        std::cout << "tan(1*pi/4) = " << std::tan(1*pi/4) << std::endl // 45°
                  << "tan(3*pi/4) = " << std::tan(3*pi/4) << std::endl // 135°
                  << "tan(5*pi/4) = " << std::tan(5*pi/4) << std::endl // -135°
                  << "tan(7*pi/4) = " << std::tan(7*pi/4) << std::endl; // -45°
        */

        /*
        int x1 = 60;
        int y1 = 60;
        int x2 = 300;
        int y2 = 360;
        int dx = (x2 - x1);
        int dy = (y2 - y1);

        int step = 0;
        if (abs(dx) >= abs(dy))
          step = abs(dx);
        else
          step = abs(dy);
        step = abs(dy);

        dx = dx / step;
        dy = dy / step;
        x = x1;
        y = y1;
        int i = 0;

        while (i <= step) {
          SDL_RenderDrawPoint(raycaster::renderer, round(x), round(y));
          x = x + dx;
          y = y + dy;
          i = i + 1;
        }
        */



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
