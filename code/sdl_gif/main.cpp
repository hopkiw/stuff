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
const int WIDTH = 99;

struct Color {
  int r, g, b;
};

class Tile {
 public:
    static const int TILE_WIDTH = 10;

    Tile(int, int, int);

    void render();

    SDL_Rect mTile;
    Color color;

    int mPosX, mPosY;
};


ostream& operator<<(ostream& os, const SDL_Rect& tile) {
    return os << "{"
              << "x: " << tile.x << ", "
              << "y: " << tile.y << ", "
              << "w: " << tile.w << ", "
              << "h: " << tile.h
              << "}";
}

bool init();
void close();

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;

Tile::Tile(int x, int y, int w)
: color({}), mPosX(x), mPosY(y) {
  mTile.x = mPosX;
  mTile.y = mPosY;
  mTile.w = w;
  mTile.h = w;
}

void Tile::render() {
  SDL_SetRenderDrawColor(gRenderer, color.r, color.g, color.b, 0xFF);
  if (SDL_RenderFillRect(gRenderer, &mTile) != 0)
    SDL_Log("error rendering tile");
}

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

/*
vector<int> parse_map(vector<string> lines) {
  vector<int> res;
  for (auto line : lines) {
    for (int i = 0; i < line.length(); i++) {
      char chr = line[i];
      switch (chr) {
        case '1':
          res.push_back(1);
          break;
        case '0':
          res.push_back(0);
          break;
        default:
          SDL_Log("unknown character in map: \"%c\"", chr);
          break;
      }
    }
  }

  return res;
}
*/

/*
void save(vector<int> map) {
  ofstream myfile;
  myfile.open("example.txt");
  for (int i = 0; i < map.size(); i++) {
    if (map[i] == 1) {
      myfile << '1';
    } else {
      myfile << '0';
    }
  if ((i+1) % WIDTH == 0)
    myfile << '\n';
  }
  myfile.close();

  return;
}
*/

struct Camera {
  int x;
  int y;
  int z;
};

int main(int argc, char* args[]) {
  if (!init()) {
    printf("Failed to initialize!\n");
    return -1;
  }
  bool quit = false;

  SDL_Event e;

  Camera camera = {0, 0, 11};

  int fh = open("myinput.gif", O_RDONLY);
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

  SDL_Log("Loaded gif %dx%d", gif->SWidth, gif->SHeight);

  int click_x = 0, click_y = 0;

  while (!quit) {
    //time_t start = clock();
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
      GifColorType color = gif->SColorMap->Colors[*src];
      SDL_SetRenderDrawColor(gRenderer, color.Red, color.Green, color.Blue, 0xFF);
      SDL_RenderDrawPoint(gRenderer, ++x + camera.x, y + camera.y);
      if (i > 0 && (i % gif->SWidth == 0)) {
        x = 0;
        ++y;
      }
      ++src;
    }
    SDL_Rect mTile;
    mTile.x = click_x;
    mTile.y = click_y;
    mTile.w = 10;
    mTile.h = 10;
    if (click_x != 0 || click_y != 0) {
      SDL_SetRenderDrawColor(gRenderer, 0xFF, 0, 0, 0xFF);
      SDL_RenderFillRect(gRenderer, &mTile);
    }
    SDL_RenderPresent(gRenderer);
  }

  close();

  return 0;
}
