#include <SDL2/SDL.h>
#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <gif_lib.h>

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

  int fh = open("/home/cc/7sd.gif", O_RDONLY);
  if (fh == -1) {
    perror("error opening input gif");
    return 1;
  }

  int err;
  GifFileType *gif = DGifOpenFileHandle(fh, &err);
  if (DGifSlurp(gif) != GIF_OK) {
    cout << "GIF_ERR" << endl;
    return 1;
  }
  close(fh);

  SDL_Log("Loaded GIF %dx%d", gif->SWidth, gif->SHeight);

  int click_x = 0, click_y = 0;

  bool fill = false;
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
        case SDL_MOUSEBUTTONDOWN:
          if (e.button.button == SDL_BUTTON_LEFT) {
            click_x = e.button.x - camera.x;
            click_y = e.button.y - camera.y;
            fill = true;
            SDL_Log("clicked (%d,%d)", click_x, click_y);
          }
          break;
      }
    }

    if (fill == true) {
      fill = false;
      int idx = (click_y * gif->SWidth) + click_x;
      int pixelcount = gif->SWidth * gif->SHeight;
      int replace = *(gif->SavedImages->RasterBits + idx);

      set<int> pixels;
      pixels.insert(idx);
      while (pixels.size() != 0) {
        auto it = pixels.begin();
        if (gif->SavedImages->RasterBits[*it] == 0) {
          break;
        }
        gif->SavedImages->RasterBits[*it] = 0;

        vector<int> neighbors = getneighbors(*it, gif->SWidth, pixelcount);
        for (auto i : neighbors) {
          if (gif->SavedImages->RasterBits[i] == replace)
            pixels.insert(i);
        }

        pixels.erase(it);
      }
    }

    // Non-white background
    SDL_SetRenderDrawColor(gRenderer, 0xBA, 0xBA, 0xBA, 0xBA);
    SDL_RenderClear(gRenderer);

    GifByteType* src = gif->SavedImages->RasterBits;
    int x = 0, y = 0;
    int pixels = gif->SWidth * gif->SHeight;
    for (int i = 0; i < pixels; ++i) {
      GifColorType color = gif->SColorMap->Colors[*src];
      SDL_SetRenderDrawColor(gRenderer, color.Red, color.Green, color.Blue, 0);
      SDL_RenderDrawPoint(gRenderer, ++x + camera.x, y + camera.y);
      if (i > 0 && (i % gif->SWidth == 0)) {
        x = 0;
        ++y;
      }
      ++src;
    }
    SDL_RenderPresent(gRenderer);
  }

  close();

  return 0;
}
