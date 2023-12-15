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

struct Color {
  int r, g, b;
};

Color hslToRgb(int hin, float s, float l) {
  if (hin < 0 || hin > 360)
    SDL_Log("error 1");
  if (s < 0 || s > 1)
    SDL_Log("error 2");
  if (l < 0 || l > 1)
    SDL_Log("error 3");

  float c = (1 - abs((2 * l) - 1)) * s;
  float h = hin / 60.0;
  float x = c * (1 - abs(fmod(h, 2) - 1));
  float m = l - (0.5 * c);

  struct {
    float r, g, b;
  } res {};

  if        (h < 1 && h > 0) {
    res = {c, x, 0};
  } else if (h < 2 && h > 1) {
    res = {x, c, 0};
  } else if (h < 3 && h > 2) {
    res = {0, c, x};
  } else if (h < 4 && h > 3) {
    res = {0, x, c};
  } else if (h < 5 && h > 4) {
    res = {x, 0, c};
  } else if (h < 6 && h > 5) {
    res = {c, 0, x};
  }

  res = {
    (res.r + m) * 255,
    (res.g + m) * 255,
    (res.b + m) * 255};

  Color rres;
  rres.r = static_cast<int>(res.r);
  rres.g = static_cast<int>(res.g);
  rres.b = static_cast<int>(res.b);

  return rres;
}

int main(int argc, char* args[]) {
  if (!init()) {
    printf("Failed to initialize!\n");
    return -1;
  }
  bool quit = false;

  SDL_Event e;
  // Color color = hslToRgb(30, 95.8, 52.9);
  // Color color = hslToRgb(210, 0.79, 0.3);
  // SDL_Log("convert to rgb %d,%d,%d", color.r, color.g, color.b);

  // Camera camera = {0, 0, 11};
  Camera camera = {};

  int click_x = 0, click_y = 0;
  bool click = false;

  int offset = 0;
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
            click_x = e.motion.x - camera.x;
            click_y = e.motion.y - camera.y;
          }
          break;
        case SDL_MOUSEBUTTONDOWN:
          if (e.button.button == SDL_BUTTON_LEFT) {
            click = true;
            click_x = e.button.x - camera.x;
            click_y = e.button.y - camera.y;
            SDL_Log("clicked (%d,%d)", click_x, click_y);
          }
          break;
        case SDL_MOUSEBUTTONUP:
          if (e.button.button == SDL_BUTTON_LEFT)
            click = false;
          break;
      }
    }

    // Non-white background
    SDL_SetRenderDrawColor(gRenderer, 0xBA, 0xBA, 0xBA, 0xBA);
    SDL_RenderClear(gRenderer);

    int x = 0, y = 0;
    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0);
    for (int i = 0; i < (400 * 400); i++) {
      SDL_RenderDrawPoint(gRenderer, x++ + camera.x, y + camera.y);
      if (i % 400 == 0) {
        x = 0;
        ++y;
        int hue = (y + offset) % 360;
        // int hue = y % 360;
        if (hue % 60 == 0)
          ++hue;
        Color cnew = hslToRgb(hue, 1, 0.5);
        SDL_SetRenderDrawColor(gRenderer, cnew.r, cnew.g, cnew.b, 0);
      }
    }

    SDL_RenderPresent(gRenderer);
    ++offset;
  }

  close();

  return 0;
}
