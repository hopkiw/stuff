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

const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 480;
// const int WIDTH = 99;


struct Particle {
  int x, y, v;

  Particle(int, int, int);
  Particle() : x(0), y(0), v(0) {}
};

Particle::Particle(int x, int y, int v) : x{x}, y{y}, v{v} {}

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

int main(int argc, char* args[]) {
  if (!init()) {
    printf("Failed to initialize!\n");
    return -1;
  }
  bool quit = false;

  SDL_Event e;

  // Camera camera = {0, 0, 11};
  Camera camera = {};

  int click_x = 0, click_y = 0;

  vector<Particle> particles;

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
          }
          break;
        case SDL_MOUSEMOTION:
          if (e.motion.state & SDL_BUTTON_LMASK) {
            click_x = e.button.x - camera.x;
            click_y = e.button.y - camera.y;
            SDL_Log("clicked (%d,%d)", click_x, click_y);
            Particle particle = {click_x, click_y, -9};
            particles.push_back(particle);
          }
          break;
      }
    }

    // Non-white background
    SDL_SetRenderDrawColor(gRenderer, 0xBA, 0xBA, 0xBA, 0xBA);
    SDL_RenderClear(gRenderer);

    // Black box
    SDL_Rect bg;
    bg.x = 0;
    bg.y = 0;
    bg.w = 400;
    bg.h = 400;
    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0);
    SDL_RenderFillRect(gRenderer, &bg);

    // Tiles
    vector<SDL_Rect> tiles;
    for ( vector<Particle>::iterator it = particles.begin();
        it != particles.end(); ) {
      it->v += 1;
      it->y += it->v;

      if (it->y >= 400) {
        it = particles.erase(it);
      } else {
        SDL_Rect mTile;
        mTile.x = it->x;
        mTile.y = it->y;
        mTile.h = 10;
        mTile.w = 10;

        tiles.push_back(mTile);

        ++it;
      }
    }

    for (auto tile : tiles) {
        SDL_SetRenderDrawColor(gRenderer, 0xFF, 0, 0, 0);
        SDL_RenderFillRect(gRenderer, &tile);
    }

    SDL_RenderPresent(gRenderer);
  }

  close();

  return 0;
}
