#include <SDL2/SDL.h>
#include <stdio.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
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

  std::vector<int> maze;  // aka 'visited'
  std::vector<int> walls;

  Camera camera = {0, 0, 24};
  int idx = rand() % realmap.size();

  maze.push_back(idx);
  SDL_Log("first cell %d", idx);

  if (idx > 1 && idx % WIDTH != 0) {
    walls.push_back(idx - 1);  // left
  }

  if (idx < realmap.size() && ((idx + 1) % WIDTH != 0)) {
    walls.push_back(idx + 1);  // right
  }

  if (idx > WIDTH) {
    walls.push_back(idx - WIDTH);  // up
  }

  if (idx + WIDTH < realmap.size()) {
    walls.push_back(idx + WIDTH);  // down
  }

  for (int i = 0; i < maze.size(); i++) {
    realmap[maze[i]] = 1;
  }

  /*
  for (int i = 0; i < walls.size(); i++) {
    realmap[walls[i]] = 2;
  }
  */

  bool next;
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
              next = true;
              break;
          }
          // SDL_Log("camera (%d,%d,%d)", camera.x, camera.y, camera.z);
          break;
        case SDL_MOUSEBUTTONDOWN:
          if (e.button.button == SDL_BUTTON_LEFT) {
            int tilex = (e.button.x / camera.z) + camera.x;
            int tiley = (e.button.y / camera.z) + camera.y;
            int tileidx = (tiley * WIDTH) + (tilex % WIDTH);
            SDL_Log("clicked %d (%d,%d)", tileidx, tilex, tiley);
            if (tileidx < realmap.size())
              realmap[tileidx] = !realmap[tileidx];
          }
          break;
      }
    }

    if (next == true) {
      next = false;

      // pick random wall
      idx = rand() % walls.size();
      idx = walls[idx];
      SDL_Log("next cell %d", idx);

      int visited = 0;
      int idxn = 0;
      vector<int>::iterator it;

      if (idx > 1 && idx % WIDTH != 0) {
        it = find(maze.begin(), maze.end(), idx - 1);
        if (it != maze.end()) {
          idxn = idx + 1;
          visited++;
        }
      }

      if (idx < realmap.size() && ((idx + 1) % WIDTH != 0)) {
        it = find(maze.begin(), maze.end(), idx + 1);
        if (it != maze.end()) {
          idxn = idx - 1;
          visited++;
        }
      }

      if (idx > WIDTH) {
        it = find(maze.begin(), maze.end(), idx - WIDTH);
        if (it != maze.end()) {
          idxn = idx + WIDTH;
          visited++;
        }
      }

      if (idx + WIDTH < realmap.size()) {
        it = find(maze.begin(), maze.end(), idx + WIDTH);
        if (it != maze.end()) {
          idxn = idx - WIDTH;
          visited++;
        }
      }

      SDL_Log("%d squares have been visited adjacent to idx %d, idxn %d",
          visited, idx, idxn);

      if (visited == 1) {
        maze.push_back(idx);
        maze.push_back(idxn);
        cout << "maze: ";
        for (int i = 0; i < maze.size(); i++) {
          cout << maze[i] << " ";
        }
        cout << endl;

        it = find(walls.begin(), walls.end(), idxn + 1);
        if (it == walls.end()) {
          it = find(maze.begin(), maze.end(), idxn + 1);
          if (it == maze.end())
            walls.push_back(idxn + 1);
        }

        it = find(walls.begin(), walls.end(), idxn - 1);
        if (it == walls.end()) {
          it = find(maze.begin(), maze.end(), idxn - 1);
          if (it == maze.end())
            walls.push_back(idxn - 1);
        }

        it = find(walls.begin(), walls.end(), idxn + WIDTH);
        if (it == walls.end()) {
          it = find(maze.begin(), maze.end(), idxn + WIDTH);
          if (it == maze.end())
            walls.push_back(idxn + WIDTH);
        }

        it = find(walls.begin(), walls.end(), idxn - WIDTH);
        if (it == walls.end()) {
          it = find(maze.begin(), maze.end(), idxn - WIDTH);
          if (it == maze.end())
            walls.push_back(idxn - WIDTH);
        }

        it = find(walls.begin(), walls.end(), idxn);
        if (it != walls.end()) {
          SDL_Log("erased idxn %d", idxn);
          walls.erase(it);
        }
      }

      it = find(walls.begin(), walls.end(), idx);
      if (it != walls.end()) {
        SDL_Log("erased idx %d", idx);
        walls.erase(it);
      }

      for (int i = 0; i < maze.size(); i++) {
        realmap[maze[i]] = 1;
      }
      cout << "walls: ";
      for (int i = 0; i < walls.size(); i++) {
        cout << walls[i] << " ";
        // realmap[walls[i]] = 2;
      }
      cout << endl;
    }

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
      } else if (realmap[i] == 2) {
        t.color = {0xEE, 0xD2, 0x02};
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

// TODO(Liam):
//
// generate maps in this form
// visualize A* pathing algorithm
