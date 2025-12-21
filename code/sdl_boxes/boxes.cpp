// Copyright 2025 Liam Hopkins
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>


namespace boxes {

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

}  // namespace boxes

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
    using boxes::renderer;
    using boxes::win;

    if (!boxes::Init()) {
        std::cout << "genuinely, wtf" << std::endl;
        return 7;
    }

    SDL_Color bgColor = {0, 0, 0, 0xFF};

    SDL_Rect center = {
        (boxes::SCREEN_WIDTH / 2),
        (boxes::SCREEN_HEIGHT / 2),
        8, 8};

    int width = 38;
    int height = 25;

    typedef struct Rectangle {
        int x;
        int y;
    } Rectangle;

    std::vector<Rectangle> rectangles;
    rectangles.push_back({width, height});
    Uint32 currentTicks = SDL_GetTicks();
    Uint32 lastTicks = currentTicks;
    // Uint32 lastCreatedTicks = currentTicks;

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
            }
        }

        currentTicks = SDL_GetTicks();
        Uint32 delta = currentTicks - lastTicks;

        SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 0xFF, 0, 0, 0xFF);
        SDL_RenderDrawRect(renderer, &center);

        for (auto it = rectangles.begin(); it != rectangles.end(); ++it) {
            SDL_Rect r = {
                center.x - (it->x / 2),
                center.y - (it->y / 2),
                it->x, it->y};
            SDL_SetRenderDrawColor(renderer, 0, 0xFF, 0, 0xFF);
            SDL_RenderDrawRect(renderer, &r);
        }

        for (auto it = rectangles.begin(); it != rectangles.end();) {
            it->x *= (0.01 * delta);
            it->y *= (0.01 * delta);
            if (it->x > boxes::SCREEN_WIDTH || it->y > boxes::SCREEN_HEIGHT) {
                it = rectangles.erase(it);
            } else {
                ++it;
            }
        }
        if (rectangles.size() > 1) {
            float ratio = rectangles[1].x / rectangles[0].x;
            std::cout << "ratio is " << ratio << std::endl;
            if (ratio >= 1.25)
                rectangles.push_back({width, height});
        }
        SDL_RenderPresent(renderer);
        lastTicks = currentTicks;
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);

    SDL_Quit();
    return 0;
}
