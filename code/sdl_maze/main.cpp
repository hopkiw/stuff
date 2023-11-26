#include <SDL2/SDL.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 480;

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

bool init();

void close();

SDL_Window* gWindow = NULL;

SDL_Renderer* gRenderer = NULL;

Tile::Tile(int x, int y, int w) : color({}), mPosX(x), mPosY(y)  {
  mTile.x = mPosX;
  mTile.y = mPosY;
  mTile.w = w;
  mTile.h = w;
}

void Tile::render() {
  SDL_SetRenderDrawColor(gRenderer, color.r, color.g, color.b, 0xFF);
  SDL_RenderFillRect(gRenderer, &mTile);
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

vector<vector<int>> parse_map(vector<string> lines) {
  vector<vector<int>> res;
  for (auto line : lines) {
    vector<int> linev;
    for (int i = 0; i < line.length(); i++) {
      char chr = line[i];
      switch (chr) {
        case '1':
          linev.push_back(1);
          break;
        case '0':
          linev.push_back(0);
          break;
        default:
          SDL_Log("unknown character in map: \"%c\"", chr);
          break;
      }
    }
    res.push_back(linev);
    linev.clear();
  }

  return res;
}

void save(vector<vector<int>> map) {
  ofstream myfile;
  myfile.open("example.txt");
  for (auto line : map) {
    for (auto tile : line) {
      if (tile == 1) {
        myfile << '1';
      } else {
        myfile << '0';
      }
    }
    myfile << '\n';
  }
  myfile.close();

  return;
}

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

  std::vector<Tile> tiles;

  string map[] = {
    "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000",
    "011111111111010111011111111111111101111101111111110111111101111111111101111111111111110111110101010",
    "000100010101010001010101000101000100010001010000000000010001000000000000000100010100000001000101010",
    "011111010101011111110101010101010111011101010111011101011111011101111101010111010111111101110111010",
    "000100010100010000000000010000010000010001000100000101010000010000010001010100000000000001000100010",
    "011101010101110101010111010111110101011101111111010111010101110101010111111101011101111101111101110",
    "000101010101010101010100010001000101010001000000010100010101010101010001010101000101000101010001000",
    "010111010101010111011101110111111111111111011111111111110111011111110101010111010111110101011111110",
    "010100000000010100010001010000010000000000000100000000010100010001000101010001010001000001000000000",
    "010101011101010111111111010101110101111101010101011111011101110111010111011101110111111111111111110",
    "010101000101010101000000000101010100010001010101000100000000010100010101000000000001000101000000010",
    "010101011111111101110111011101010111011101011101011101111111110111010101110111010101110101111111010",
    "010101000101000000000100010001010100010001010001010001010000010101010100010001010101000000010000010",
    "010111010101111111111111010111111111110101010111111111011101111101111101010111111111011111011101010",
    "010100010001000000010000010001000000000101010100000000000000000101000101000001000000010000000001000",
    "010101111101011111011111110111111111110111111111111111111101110101011111111111011111111111111101110",
    "010100010001010000000100000100000001000100000100000000000001000000010100010101010000010000000001000",
    "011111111111111111110101110101110111011101110111110111110111110101010111010101111111010111111111110",
    "010100000000000000000001000101000000010001000100000101000001010101000100010000000001000100000000010",
    "010111010111011101011111110101010101111101110101111101110111011111010101011111111101010101111111010",
    "000100010100010101010000000101010101010001000001000000000100000000010001000000000100010100010100000",
    "010111111111010101111111011101010111010101010111010111111111010111011101111111110111111111110111110",
    "010100000000010001000000010001010100000101010001010100010000010001010001000000010000000001000000000",
    "011101010111011111111101111111111111111111010101111111011111111101111111111111010111111101111111110",
    "010101010100010100010101000100010000000101010100010001000000000001000100010000000001010101000000000",
    "010111111101010111010111110101011101110101111111111101011101111111110111011111110111010101110101110",
    "000100000001010000010101010001000101010001010100000000010000010000000100010000000001000000010101000",
    "010101111111111101010101011111110111011101010111111101111111111111110101011111111111111111010111110",
    "010101010101010001010001010000010001000001000000000001000100010000010101000101000101000000000100010",
    "011111010101010111010111011111010101011101110111111111110101011111010111110101110101111101110101010",
    "010000010001010100000100000101010100010000000101000001000001000000000101000000000000000001000101000",
    "010111010111011111011101110101011101111101111101110101010111110111110101111111111111111111111111110",
    "010100010000010100010001000001010000010001000100010101010101000100000001010001000100000101000101000",
    "011111011111010111111111110111011111111111110111010111111101111111110111011101011111110101110101110",
    "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000",
  };

  vector<string> mapv(begin(map), end(map));
  vector<vector<int>> realmap = parse_map(mapv);

  Camera camera = {0, 0, 14};
  // bool setup;
  while (!quit) {
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
            case 's':
              save(realmap);
              continue;
            case SDLK_UP:
              ++camera.z;
              break;
            case SDLK_DOWN:
              --camera.z;
              break;
            case 'h':
              --camera.x;
              // if (camera.x < 0)
              //  camera.x = 0;
              break;
            case 'j':
              ++camera.y;
              break;
            case 'k':
              --camera.y;
              break;
            case 'l' :
              ++camera.x;
              break;
          }
          SDL_Log("camera (%d,%d,%d)", camera.x, camera.y, camera.z);
          break;
        case SDL_MOUSEBUTTONDOWN:
          if (e.button.button == SDL_BUTTON_LEFT) {
            int tilex = (e.button.x / camera.z) + camera.x;
            int tiley = (e.button.y / camera.z) + camera.y;
            if (tiley < realmap.size() && tilex < realmap[0].size())
              realmap[tiley][tilex] = !realmap[tiley][tilex];
            // int idx = (tiley * realmap[0].size()) + tilex;

            /*
            if (idx < tiles.size()) {
              SDL_Log("swap tile %d,%d", tilex, tiley);
              if (tiles[idx].color.r == 0xFF) {
                tiles[idx].color = {};
              } else {
                tiles[idx].color = {0xFF, 0xFF, 0xFF};
              }
            }
            */
          }
          break;
      }
    }

    int x = 0;
    int y = 0;
    tiles.clear();
    for (auto line : realmap) {
      for (auto i : line) {
        Tile t(
            x - (camera.z * camera.x),
            y - (camera.z * camera.y),
                 camera.z);
        if (i)
          t.color = {0xFF, 0xFF, 0xFF};
        tiles.push_back(t);

        x += camera.z;
      }
      x = 0;
      y += camera.z;
    }
    tiles.back().color = {0xFF, 0, 0};
    // setup = true;

    // Non-white background
    SDL_SetRenderDrawColor(gRenderer, 0xBA, 0xBA, 0xBA, 0xBA);
    SDL_RenderClear(gRenderer);

    // Render tiles
    for (auto tile : tiles) {
      tile.render();
    }

    SDL_RenderPresent(gRenderer);
  }

  close();

  return 0;
}

// TODO(Liam): zoomable map
//
// add grid, add edit
