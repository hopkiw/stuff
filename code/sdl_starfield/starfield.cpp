// Copyright 2025 Liam Hopkins
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>


namespace starfield {

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

SDL_Window* win = NULL;
SDL_Renderer* renderer = NULL;
// TTF_Font* font = NULL;

bool Init() {
    srand(time(NULL));

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

}  // namespace starfield

typedef struct Point {
    int x;
    int y;
} Point;

typedef struct Star {
    Point p;
    int size;
    float vel;
} Star;

std::ostream& operator<<(std::ostream& os, const Point& p) {
    return os << "{" << p.x << "," << p.y << "}";
}


int main() {
    if (!starfield::Init())
        return 1;

    const SDL_Color colors[] = {
        {0xFF, 0x00, 0x00, 0xFF},
        {0x00, 0xFF, 0x00, 0xFF},
        {0x00, 0x00, 0xFF, 0xFF},
        {0xFF, 0xFF, 0x00, 0xFF},
        {0x00, 0xFF, 0xFF, 0xFF},
    };

    const SDL_Color bgColor{0x0, 0x0, 0x0, 0xFF};
    const Point center = {starfield::SCREEN_WIDTH / 2, starfield::SCREEN_HEIGHT / 2};
    std::cout << "center at " << center << std::endl;

    std::vector<Star> stars;
    float circleAngle = 0.0;
    for (int i = 0; i < 15; ++i) {
        Point p = {rand() % starfield::SCREEN_WIDTH, rand() % starfield::SCREEN_HEIGHT};
        /*
        Point p = {
            center.x + static_cast<int>(140 * sin(circleAngle)),
            center.y - static_cast<int>(140 * cos(circleAngle))
        };
        */
        Star s = {p, 8, 0.08};
        stars.push_back(s);
        circleAngle += 0.5;
    }

    Uint32 lastTicks = SDL_GetTicks();

    bool pause = true;
    bool print = false;
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
                        case SDLK_SPACE:
                            pause = !pause;
                            break;
                        case 'j':
                            std::cout << "print true" << std::endl;
                            print = true;
                            break;
                    }
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    if (e.motion.state & SDL_BUTTON_LMASK) {
                        std::cout << "clicked at " << Point{e.motion.x, e.motion.y} << std::endl;
                    }
                    break;
            }
        }

        Uint32 currentTicks = SDL_GetTicks();
        float delta = currentTicks - lastTicks;

        SDL_SetRenderDrawColor(starfield::renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
        SDL_RenderClear(starfield::renderer);

        auto idx = 0;
        for (auto it = stars.begin(); it != stars.end(); ++it) {
            if (it->p.x < 0 || it->p.x > starfield::SCREEN_WIDTH) {
                if (print)
                    std::cout << "x out of bounds: " << it->p.x << std::endl;
                // continue;
                it->p = {rand() % starfield::SCREEN_WIDTH, rand() % starfield::SCREEN_HEIGHT};
            }

            if (it->p.y < 0 || it->p.y > starfield::SCREEN_HEIGHT) {
                if (print)
                    std::cout << "y out of bounds: " << it->p.y << std::endl;
                // continue;
                it->p = {rand() % starfield::SCREEN_WIDTH, rand() % starfield::SCREEN_HEIGHT};
            }

            SDL_Color c = colors[idx];
            SDL_Rect r = {it->p.x, it->p.y, it->size, it->size};
            SDL_SetRenderDrawColor(starfield::renderer, c.r, c.g, c.b, c.a);
            SDL_RenderFillRect(starfield::renderer, &r);
            idx = (idx + 1) % 5;
        }

        for (auto it = stars.begin(); it != stars.end(); ++it) {
            float width = it->p.x - center.x;
            float height = it->p.y - center.y;

            if (width == 0 || height == 0)
                continue;

            if (print) {
                std::cout << "test star " << it->p << std::endl;
                std::cout << "width: " << width << " height: " << height << std::endl;
            }

            float distance = sqrt((width * width) + (height * height));
            distance += it->vel * delta;

            float angle = atan(height / width);
            float newWidth = distance * cos(angle);
            float newHeight = distance * sin(angle);

            if (print) {
                std::cout << "new distance to star: " << distance << std::endl;
                std::cout << "at angle: " << angle << std::endl;
                std::cout << "newWidth: " << newWidth << std::endl;
                std::cout << "newHeight: " << newHeight << std::endl;
            }

            if (it->p.x > center.x)
                it->p.x = center.x + abs(newWidth);
            else
                it->p.x = center.x - abs(newWidth);

            if (it->p.y > center.y)
                it->p.y = center.y + abs(newHeight);
            else
                it->p.y = center.y - abs(newHeight);

            /*
            if (print)
                std::cout << "next star at " << Point{r.x, r.y} << std::endl;

            SDL_SetRenderDrawColor(starfield::renderer, 0xFF, 0xFF, 0xFF, 0xFF);
            SDL_RenderFillRect(starfield::renderer, &r);

            SDL_SetRenderDrawColor(starfield::renderer, 0xFF, 0x0, 0x0, 0xFF);
            SDL_RenderDrawLine(starfield::renderer, it->p.x, it->p.y, r.x, r.y);
            */
        }

        print = false;

        SDL_RenderPresent(starfield::renderer);
        lastTicks = currentTicks;
    }

    SDL_DestroyRenderer(starfield::renderer);
    SDL_DestroyWindow(starfield::win);

    SDL_Quit();
    return 0;
}
