// Copyright 2025 Liam Hopkins
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>


namespace tracker {

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

SDL_Window* win = NULL;
SDL_Renderer* renderer = NULL;
TTF_Font* font = NULL;

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

    font = TTF_OpenFont("/usr/share/fonts/truetype/terminus/TerminusTTF-4.46.0.ttf", 20);
    if (font == NULL) {
        std::cout << "Font could not be created. TTF Error: " << TTF_GetError() << std::endl;
        return false;
    }

    return true;
}

class ScrollBox {
 public:
    ScrollBox(int, int, int, int);
    void Draw();
    void ScrollDown();
    void ScrollUp();

 private:
    int highlight = 0;
    int x, y, w, h;
};

ScrollBox::ScrollBox(int x_, int y_, int w_, int h_) : x{x_}, y{y_}, w{w_}, h{h_}  { }

void ScrollBox::Draw() {
    const SDL_Color darkGrey{0x2c, 0x2c, 0x2c, 0xFF};
    const SDL_Color lightGrey{0x27, 0x27, 0x27, 0xFF};
    const SDL_Color teal{0x2b, 0x3e, 0x3a, 0xFF};

    SDL_Rect r = {x, y, w, h};
    SDL_SetRenderDrawColor(renderer, darkGrey.r, darkGrey.g, darkGrey.b, darkGrey.a);
    SDL_RenderFillRect(renderer, &r);

    int space = 1240 / 4;
    SDL_Color color = {0xFF, 0xFF, 0xFF, 0xFF};
    SDL_Surface* mySurface = TTF_RenderText_Solid(font, "00 NOTE 00 000", color);
    SDL_Texture* myTexture = SDL_CreateTextureFromSurface(renderer, mySurface);
    for (int i = 0; i < 18; ++i) {
        if (i == highlight) {
            SDL_SetRenderDrawColor(renderer, teal.r, teal.g, teal.b, teal.a);
            SDL_Rect r2 = {
                x + 10,
                y + 10 + i * mySurface->h,
                w,
                mySurface->h};
            SDL_RenderFillRect(renderer, &r2);
        }
        for (int j = 0; j < 4; ++j) {
            SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
            SDL_Rect textRect = {
                x + 10 + space * j,
                y + 10 + i * mySurface->h,
                mySurface->w,
                mySurface->h,
            };

            SDL_RenderCopy(renderer, myTexture, NULL, &textRect);
        }
    }

    SDL_SetRenderDrawColor(renderer, lightGrey.r, lightGrey.g, lightGrey.b, lightGrey.a);
    for (int i = 0; i < 4; ++i) {
        SDL_RenderDrawLine(renderer, x + (space * i), y + 15, x + (space * i), y + 405);
    }
}

void ScrollBox::ScrollDown() {
    ++highlight;
    if (highlight > 17)
        highlight = 0;
    std::cout << "highlight is " << highlight << std::endl;
}

void ScrollBox::ScrollUp() {
    --highlight;
    if (highlight < 0)
        highlight = 0;
    std::cout << "highlight is " << highlight << std::endl;
}


}  // namespace tracker

typedef struct Point {
    int x;
    int y;
} Point;

std::ostream& operator<<(std::ostream& os, const Point& p) {
    return os << "{" << p.x << "," << p.y << "}";
}

/*
typedef struct Channel {
    Uint8 sampleHigh : 4;
    Uint8 paramHigh : 4;
    Uint8 paramLow;
    Uint8 sampleLow : 4;
    Uint8 effectHigh : 4;
    Uint8 effectLow;
} Channel;
*/


typedef struct Division {
    std::vector<Uint32> channels;  // 1-8 channels
} Division;

typedef struct Pattern {
    Division divisions[64];
} Pattern;

typedef struct Module {
    std::vector<Pattern> patterns;
} Module;


int main() {
    using tracker::renderer;
    using tracker::win;
    using tracker::font;
    using tracker::SCREEN_WIDTH;
    using tracker::SCREEN_HEIGHT;

    if (!tracker::Init())
        return 1;




    tracker::ScrollBox sb = tracker::ScrollBox(20, 280, 1240, 480);
    const SDL_Color bgColor{0x0, 0x0, 0x0, 0xFF};

    SDL_Rect settings1 = {20, 20, 600, 240};
    SDL_Rect settings2 = {660, 20, 600, 240};

    // Uint32 lastTicks = SDL_GetTicks();
    Uint32 lastMoveTicks = SDL_GetTicks();
    bool quit = false;
    bool playing = false;
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
                        case SDLK_DOWN:
                            sb.ScrollDown();
                            break;
                        case SDLK_UP:
                            sb.ScrollUp();
                            break;
                        case SDLK_SPACE:
                            playing = !playing;
                            break;
                    }
                    break;
            }
        }

        Uint32 currentTicks = SDL_GetTicks();
        // float delta = currentTicks - lastTicks;

        SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
        SDL_RenderClear(renderer);


        SDL_SetRenderDrawColor(renderer, 0xBA, 0xBA, 0xBA, 0xFF);
        SDL_RenderFillRect(renderer, &settings1);
        SDL_RenderFillRect(renderer, &settings2);

        // litle grey bars between channels
        // 4 channels to start

        if (playing) {
            if ((currentTicks - lastMoveTicks) > 200) {
                sb.ScrollDown();
                lastMoveTicks = currentTicks;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0xFF, 0, 0xFF);
        SDL_RenderDrawRect(renderer, &settings1);
        SDL_RenderDrawRect(renderer, &settings2);

        sb.Draw();

        SDL_RenderPresent(renderer);
        // lastTicks = currentTicks;
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);

    SDL_Quit();
    return 0;
}

/*
    Effects table:

    [0]: Arpeggio
    [1]: Slide up
    [2]: Slide down
    [3]: Slide to note
    [4]: Vibrato
    [5]: Continue 'Slide to note', but also do Volume slide
    [6]: Continue 'Vibrato', but also do Volume slide
    [7]: Tremolo
    [8]: (Set panning position)
    [9]: Set sample offset
    [10]: Volume slide
    [11]: Position Jump
    [12]: Set volume
    [13]: Pattern Break
    [14][0]: Set filter on/off
    [14][1]: Fineslide up
    [14][2]: Fineslide down
    [14][3]: Set glissando on/off
    [14][4]: Set vibrato waveform
    [14][5]: Set finetune value
    [14][6]: Loop pattern
    [14][7]: Set tremolo waveform
    [14][8]: -- Unused --
    [14][9]: Retrigger sample
    [14][10]: Fine volume slide up
    [14][11]: Fine volume slide down
    [14][12]: Cut sample
    [14][13]: Delay sample
    [14][14]: Delay pattern
    [14][15]: Invert loop
    [15]: Set speed
*/
