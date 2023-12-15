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
#include <array>
#include <vector>

using namespace std;

const int SCREEN_WIDTH = 480;
const int SCREEN_HEIGHT = 480;
// const int WIDTH = 99;

struct Color {
  int r, g, b;
};

struct HSL {
  int h;
  float s, l;
};

HSL rgb2hsl(int r, int g, int b) {
  // printf("%d %d %d\n", r, g, b);

  float _r = r / 255.0;
  float _g = g / 255.0;
  float _b = b / 255.0;

  // printf("%f %f %f\n", _r, _g, _b);

  float max = _r;
  char s = 'r';
  if (_g > max) {
    s = 'g';
    max = _g;
  }
  if (_b > max) {
    s = 'b';
    max = _b;
  }

  float min = _r;
  if (min > _g)
    min = _g;
  if (min > _b)
    min = _b;

  // printf("max is %f, min is %f\n", max, min);

  float hue;
  float c = max - min;
  float luminance = (max + min) / 2;

  // printf("luminance is %f\n", luminance);

  float saturation;
  if (luminance <= 0.5) {
    saturation = (c) / (max + min);
  } else {
    saturation = (c) / (2.0 - c);
  }
  // printf("saturation is %f\n", saturation);

  if (c == 0) {
    HSL res = {0, saturation, luminance};
    return res;
  }

  float segment;
  switch (s) {
    case 'r':
      hue = (_g - _b) / c;
      break;
    case 'g':
      segment = (_b - _r) / c;
      hue = segment + 2;
      break;
    case 'b':
      segment = (_r - _g) / c;
      hue = segment + 4;
      break;
  }
  int h = (hue * 60) + 0.5;

  HSL res = {h, saturation, luminance};
    
  // printf("hue is %d\n", h);

  return res;
}


Color hslToRgb(int hin, float s, float l) {
  /*
  if (hin < 0 || hin > 360)
    SDL_Log("error 1 value %d", hin);
  if (s < 0 || s > 1)
    SDL_Log("error 2");
  if (l < 0 || l > 1)
    SDL_Log("error 3");
  */

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
  HSL h = rgb2hsl(255, 255, 255);
  printf("got %d\n", h.h);
  // return 0;
  if (!init()) {
    printf("Failed to initialize!\n");
    return -1;
  }
  bool quit = false;

  SDL_Event e;

  // Camera camera = {0, 0, 11};
  Camera camera = {};

  int fh = open("test.gif", O_RDONLY);
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
  int offset = 0;

  vector<Color> shiftedcolors;
  for (int i = 0; i < gif->SColorMap->ColorCount; ++i) {
    GifColorType *color = gif->SColorMap->Colors + i;
    HSL hsl = rgb2hsl(color->Red, color->Green, color->Blue);
    Color shifted = hslToRgb(hsl.h + 20, hsl.s, hsl.l);
    shiftedcolors.push_back(shifted);
  }

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
            SDL_Log("clicked (%d,%d)", click_x, click_y);
          }
          break;
      }
    }

    // Non-white background
    SDL_SetRenderDrawColor(gRenderer, 0xBA, 0xBA, 0xBA, 0xBA);
    SDL_RenderClear(gRenderer);

    GifByteType* src = gif->SavedImages->RasterBits;
    int x = 0, y = 0;
    int pixels = gif->SWidth * gif->SHeight;
    for (int i = 0; i < pixels; ++i) {
      /*
      Color color = shiftedcolors[*src];
      SDL_SetRenderDrawColor(gRenderer, color.r, color.g, color.b, 0);
      */
      // TODO: pre-calculate all necessary colors, not on the fly
      GifColorType color = gif->SColorMap->Colors[*src];
      
      HSL h = rgb2hsl(color.Red, color.Green, color.Blue);
      h.h += offset;
      h.h += y;
      h.h = h.h % 360;
      if (h.h % 60 == 0)
        ++h.h;
      Color shifted = hslToRgb(h.h, h.s, h.l);
      SDL_SetRenderDrawColor(gRenderer, shifted.r, shifted.g, shifted.b, 0);

      SDL_RenderDrawPoint(gRenderer, ++x + camera.x, y + camera.y);
      if (i > 0 && (i % gif->SWidth == 0)) {
        x = 0;
        ++y;
      }
      ++src;
    }

    SDL_RenderPresent(gRenderer);
    offset += 4;
  }

  close();

  return 0;
}
