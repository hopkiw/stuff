#include <SDL2/SDL.h>
#include <stdio.h>
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
  std::ifstream myfile("test.txt");
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

  Camera camera = {0, 0, 11};
  bool run = false;

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
              run = true;
              break;
          }
          break;
        case SDL_MOUSEBUTTONDOWN:
          if (e.button.button == SDL_BUTTON_LEFT) {
            int tilex = (e.button.x / camera.z) + camera.x;
            int tiley = (e.button.y / camera.z) + camera.y;
            int tileidx = (tiley * WIDTH) + (tilex % WIDTH);
          }
          break;
      }
    }

    // Manipulate map
    // Any live cell with fewer than two live neighbors dies
    // Any live cell with 2 or 3 live neighbors remains alive
    // Any live cell with more than 3 live neighbors dies
    // Any dead cell with 3 live neighbors is born

    if (run == true) {
      run = false;
      vector<int> realmap2(realmap);
      for (int i = 0; i < realmap.size(); i++) {
        int neighbors = 0;

        // left
        if (i != 0 && ((i % WIDTH) != 0)) {
          if (realmap[i - 1] == 0) {
            neighbors++;
          }
        }

        // right
        if (i + 1 != realmap.size() && ((i + 1) % WIDTH) != 0) {
          if (realmap[i + 1] == 0) {
            neighbors++;
          }
        }

        // up
        if (i - WIDTH >= 0) {
          if (realmap[i - WIDTH] == 0) {
            neighbors++;
          }

          // up-left
          if (i % WIDTH != 0) {
            if (realmap[i - WIDTH - 1] == 0) {
              neighbors++;
            }
          }

          // up-right
          if ((i + 1) % WIDTH != 0) {
            if (realmap[i - WIDTH + 1] == 0) {
              neighbors++;
            }
          }
        }

        // down
        if (i + WIDTH < realmap.size()) {
          if (realmap[i + WIDTH] == 0) {
            neighbors++;
          }

          // down-left
          if (i % WIDTH != 0) {
            if (realmap[i + WIDTH - 1] == 0) {
              neighbors++;
            }
          }

          // down-right
          if ((i + 1) % WIDTH != 0) {
            if (realmap[i + WIDTH + 1] == 0) {
              neighbors++;
            }
          }
        }


        if (realmap[i] == 0) {
          if (neighbors < 2 || neighbors > 3) {
            realmap2[i] = 1;
            }
        } else {
          if (neighbors == 3) {
            realmap2[i] = 0;
          }
        }
      }

      realmap = realmap2;
    }

    // Generate tiles
    int x = 0;
    int y = 0;
    tiles.clear();
    for (int i = 0; i < realmap.size(); i++) {
      Tile t(
          x - (camera.z * camera.x),
          y - (camera.z * camera.y),
               camera.z);
      if (realmap[i] == 1) {
        t.color = {0xFF, 0xFF, 0xFF};
      }

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
