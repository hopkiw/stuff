#include <SDL2/SDL.h>
#include <stdio.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
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

  string map[40];
  std::ifstream myfile("final.txt");
  if (myfile.is_open()) {
    int idx = 0;
    while (myfile.good()) {
      myfile >> map[idx];
      idx++;
    }
  } else {
    cout << "Error opening map file\n";
    return 1;
  }

  vector<string> mapv(begin(map), end(map));
  vector<int> realmap = parse_map(mapv);

  Camera camera = {0, 0, 1};
  // int width = realmap[0].size();
  // int idx = rand() % realmap;
  // realmap[idx][idx] = 1;
  bool maze;
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
            case 'n':
              maze = true;
              break;
          }
          SDL_Log("camera (%d,%d,%d)", camera.x, camera.y, camera.z);
          break;
        case SDL_MOUSEBUTTONDOWN:
          if (e.button.button == SDL_BUTTON_LEFT) {
            int tilex = (e.button.x / camera.z) + camera.x;
            int tiley = (e.button.y / camera.z) + camera.y;
            int tileidx = (tiley * WIDTH) + (tilex % WIDTH);
            if (tileidx < realmap.size())
              realmap[tileidx] = !realmap[tileidx];
          }
          break;
      }
    }

    if (maze) {
      //realmap manipulation;

    }

    int x = 0;
    int y = 0;
    tiles.clear();
    for (int i = 0; i < realmap.size(); i++) {
      Tile t(
          x - (camera.z * camera.x),
          y - (camera.z * camera.y),
               camera.z);
      if (realmap[i])
        t.color = {0xFF, 0xFF, 0xFF};
      tiles.push_back(t);

      x += camera.z;

      if ((i+1) % WIDTH == 0) {
        x = 0;
        y += camera.z;
      }
    }
    tiles.back().color = {0xFF, 0, 0};

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

// TODO(Liam):
//
// generate maps in this form
// visualize A* pathing algorithm
