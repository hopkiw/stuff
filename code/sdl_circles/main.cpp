#include <SDL2/SDL.h>
#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <set>
#include <vector>

using namespace std;

const int SCREEN_WIDTH = 500;
const int SCREEN_HEIGHT = 500;
// const int WIDTH = 99;

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;

bool init() {
  // Initialization flag
  bool success = true;

  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
    return false;
  }

  // Set texture filtering to linear
  if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
    printf("Warning: Linear texture filtering not enabled!");
  }

  gWindow = SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED,
      SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
  if (gWindow == NULL) {
    printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
    success = false;
  } else {
    gRenderer = SDL_CreateRenderer(gWindow, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (gRenderer == NULL) {
      printf("Renderer could not be created! SDL Error: %s\n",
          SDL_GetError());
      success = false;
    } else {
      SDL_SetRenderDrawColor(gRenderer, 0xBA, 0xBA, 0xBA, 0xBA);
    }
  }

  return success;
}

void close() {
  SDL_DestroyRenderer(gRenderer);
  SDL_DestroyWindow(gWindow);
  gWindow = NULL;
  gRenderer = NULL;

  SDL_Quit();
}

struct Camera {
  int x;
  int y;
  // int z;
};

vector<int> getneighbors(int idx, int width, int length) {
  vector<int> res;
  // left
  if (idx != 0 && idx % width != 0) {
    res.push_back(idx - 1);
  }

  // right
  if ((idx + 1) != length && ((idx + 1) % width != 0)) {
    res.push_back(idx + 1);
  }

  // up
  if (idx - width > 0) {
    res.push_back(idx - width);
  }

  // down
  if ((idx + width) <= length) {
    res.push_back(idx + width);
  }

  return res;
}

struct Circle {
  int x, y, r;
};

int main(int argc, char* args[]) {
  if (!init()) {
    printf("Failed to initialize!\n");
    return -1;
  }
  bool quit = false;

  SDL_Event e;

  Camera camera = {};

  int click_x = 0, click_y = 0;
  bool click = false;

  vector<Circle> circles;
  int timesince = 0;
  while (!quit) {
    // time_t start = clock();
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
            case 'h':
              camera.x -= 10;
              break;
            case 'j':
              camera.y += 10;
              break;
            case 'k':
              camera.y -= 10;
              break;
            case 'l' :
              camera.x += 10;
              break;
            case 'n' :
              click = true;
              break;
          }
          break;
        case SDL_MOUSEMOTION:
          if (e.motion.state & SDL_BUTTON_LMASK) {
            click_x = e.motion.x - camera.x;
            click_y = e.motion.y - camera.y;
          }
          break;
        case SDL_MOUSEBUTTONDOWN:
          if (e.button.button == SDL_BUTTON_LEFT) {
            // click = true;
            click_x = e.button.x - camera.x;
            click_y = e.button.y - camera.y;
            SDL_Log("clicked (%d,%d)", click_x, click_y);
          }
          break;
        case SDL_MOUSEBUTTONUP:
          //if (e.button.button == SDL_BUTTON_LEFT)
          //  click = false;
          break;
      }
    }


    if (timesince == 0 && click == true) {
      for (int i = 0; i < 3; ++i) {
        // click = false;
        Circle c {100, 100, 25 + i};
        circles.push_back(c);
        Circle c2 {400, 100, 25 + i};
        circles.push_back(c2);
      }
    }
    timesince = (timesince + 1) % 50;

    // Non-white background
    SDL_SetRenderDrawColor(gRenderer, 0xBA, 0xBA, 0xBA, 0xBA);
    SDL_RenderClear(gRenderer);

    SDL_SetRenderDrawColor(gRenderer, 0xFF, 0, 0, 0);
    for (vector<Circle>::iterator it = circles.begin(); it != circles.end(); ++it) {
      for (int i = 0; i < 1440; i++) {
        float rad = i * ((2 * 3.14159) / 1440);
        int x = it->r * cos(rad);
        int y = it->r * sin(rad);

        SDL_RenderDrawPoint(gRenderer, x + it->x, y + it->y);
      }
      ++it->r;
    }

    SDL_RenderPresent(gRenderer);
  }

  close();

  return 0;
}

/*
    Displacement
    Base
    Base + Index
    Base + Displacement
    Base + Index + Displacement
    Base + (Index * Scale)
    (Index * Scale) + Displacement
    Base + (Index * Scale) + Displacement

    --

    Displacement (absolute value)
    e.g.: mov ax,[0xff] or mov [lbl],bx.

    Base (register)
    e.g.: mov ax,[bx]. absolute address in register.

    Base + Index
    e.g.: mov [ax+bx],cx. address is sum of registers.

    Base + Displacement
    e.g.: mov bx,[ax+0xcafe]. address is sum of register and value

    Base + Index + Displacement
    e.g.: mov bx,[ax+cx+0xcafe]

    Base + (Index * Scale)
    e.g.: mov ax,[di+8*si]

    (Index * Scale) + Displacement
    e.g.:

    Base + (Index * Scale) + Displacement

*/
