#include <SDL2/SDL.h>
#include <stdio.h>
#include <time.h>

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <set>
#include <vector>
#include <array>

#include "./palettes.h"

const int SCREEN_WIDTH = 1080;
const int SCREEN_HEIGHT = 720;
const int WIDTH = 10;

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;

bool operator==(const Color& lhs, const Color& rhs) {
  return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b;
}

std::vector<int> getneighbors(int idx, int width, int length) {
  std::vector<int> res;
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

class Tile {
 public:
    static const int TILE_WIDTH = 10;

    Tile(int, int, int);
    Tile(int, int, int, Color);
    Tile();

    void render();
    void move(int, int, int);
    void setcolor(Color);

 private:
    SDL_Rect mTile;
    Color color;
};

void Tile::move(int x, int y, int w) {
  mTile.x = x;
  mTile.y = y;
  mTile.w = w;
  mTile.h = w;
}

void Tile::setcolor(Color _color) {
  color = _color;
}

std::ostream& operator<<(std::ostream& os, const SDL_Rect& tile) {
    return os << "{"
              << "x: " << tile.x << ", "
              << "y: " << tile.y << ", "
              << "w: " << tile.w << ", "
              << "h: " << tile.h
              << "}";
}

Tile::Tile()
: color{} {
  mTile.x = 0;
  mTile.y = 0;
  mTile.w = 0;
  mTile.h = 0;
}

Tile::Tile(int x, int y, int w)
: color{} {
  mTile.x = x;
  mTile.y = y;
  mTile.w = w;
  mTile.h = w;
}

Tile::Tile(int x, int y, int w, Color c)
: color{c} {
  mTile.x = x;
  mTile.y = y;
  mTile.w = w;
  mTile.h = w;
}

void Tile::render() {
  SDL_SetRenderDrawColor(gRenderer, color.r, color.g, color.b, 0xFF);
  if (SDL_RenderFillRect(gRenderer, &mTile) != 0)
    SDL_Log("error rendering tile");
}

struct Camera {
  int x, y, z;
};

struct Change {
  int idx;
  Color color;
};

class TileField {
 public:
    TileField(int, int, int, int, std::array<Color, 10000>&);

    void render();
    void handle_click(int, int, bool);
    void end_event();
    void zoom(int);
    void pan(int, int);
    void play(std::vector<Change>);
    void set_color(Color c) { color = c; }

    std::vector<Change> changelist;
    Camera camera = {0, 0, 10};

 private:
    int x, y, w, h;
    int last = -1;
    Color color;
    int to_fill = -1;
    std::array<Color, 10000>& real;
};

TileField::TileField(int _x, int _y, int _w, int _h,
    std::array<Color, 10000>& _real)
: x{_x}, y{_y}, w{_w}, h{_h}, color{}, real{_real} {
  for (int i = 0; i < (100*100); ++i) {
    real[i] = Color{0xFF, 0xFF, 0xFF};
  }
}

void TileField::render() {
  int cols = w / camera.z;
  int rows = h / camera.z;

  for (int _y = 0; _y < rows; ++_y) {
    for (int _x = 0; _x < cols; ++_x) {
      int i = ((_y + camera.y) * 100) + _x + camera.x;
      Tile t = {(_x * camera.z) + x, (_y * camera.z) + y, camera.z, real[i]};
      t.render();
    }
  }
}

void TileField::handle_click(int click_x, int click_y, bool shift) {
  int tile_x = (click_x - x) / camera.z;
  int tile_y = (click_y - y) / camera.z;

  int idx = (((tile_y + camera.y) * 100) + tile_x + camera.x);
  if (idx == last)
    return;
  last = idx;

  if (shift == true) {
    to_fill = idx;
  } else {
    // "paint"
    changelist.push_back(Change{idx, real[idx]});
    real[idx] = color;
  }
}

void TileField::end_event() {
  if (to_fill == -1)
    return;

  Color replace = real[to_fill];
  std::set<int> pixels;
  pixels.insert(to_fill);
  while (pixels.size() != 0) {
    auto it = pixels.begin();
    changelist.push_back(Change{*it, real[*it]});
    real[*it] = color;
    for (auto i : getneighbors(*it, 100, 10000)) {
      if (real[i] == replace) {
        pixels.insert(i);
      }
    }
    pixels.erase(it);
  }

  to_fill = -1;
}

void TileField::zoom(int change) {
  camera.z = camera.z + change;
  if (camera.z < 4)
    camera.z = 4;
  if (camera.z > 100)
    camera.z = 100;
  pan(0, 0);
}

void TileField::pan(int change_x, int change_y) {
  camera.x = camera.x + change_x;
  if (camera.x + (w / camera.z) > 99)
    camera.x = 99 - (w / camera.z);
  if (camera.x < 0)
    camera.x = 0;

  camera.y = camera.y + change_y;
  if (camera.y + (h / camera.z) > 99)
    camera.y = 99 - (h / camera.z);
  if (camera.y < 0)
    camera.y = 0;
}

void TileField::play(std::vector<Change> changelist) {
  for (auto change : changelist) {
    real[change.idx] = change.color;
  }
}

class PaletteViewer {
 public:
    PaletteViewer(int, int);
    void render();
    void handle_click(int, int);
    const Color& get_color();
    void next();
    void prev();

 private:
    int x, y, idx = 0, pidx = 0;
};

PaletteViewer::PaletteViewer(int _x, int _y) : x{_x}, y{_y} { }

void PaletteViewer::next() {
  ++pidx;
}

void PaletteViewer::prev() {
  --pidx;
}

void PaletteViewer::render() {
  SDL_Rect options = {x, y, 400, 40};
  SDL_SetRenderDrawColor(gRenderer, 0x80, 0x80, 0x80, 0);
  SDL_RenderFillRect(gRenderer, &options);

  Palette palette = palettes[pidx];

  for (unsigned int i = 0; i < palette.size() && i < 10; ++i) {
    Color color = palette[i];
    SDL_Rect tile = {static_cast<int>(x + (40 * i)), y, 40, 40};
    SDL_SetRenderDrawColor(gRenderer, color.r, color.g, color.b, 0);
    SDL_RenderFillRect(gRenderer, &tile);
  }
}

void PaletteViewer::handle_click(int click_x, int click_y) {
  // TODO: this calculation depends on knowing the tile width
  Palette palette = palettes[pidx];
  unsigned int _idx = (click_x - x) / 40;
  if (_idx < palette.size())
    idx = _idx;
}

const Color& PaletteViewer::get_color() {
  return palettes[pidx][idx];
}

class MiniMap {
 public:
    MiniMap(int, int, const std::array<Color, 10000>&);
    void render(Camera);

 private:
    int x, y;
    const std::array<Color, 10000>& real;
};

MiniMap::MiniMap(int _x, int _y, const std::array<Color, 10000>& _real)
: x{_x}, y{_y}, real{_real} { }

void MiniMap::render(Camera c) {
  SDL_Rect bg = {x, y, 120, 120};
  SDL_SetRenderDrawColor(gRenderer, 0x80, 0x80, 0x80, 0);
  SDL_RenderFillRect(gRenderer, &bg);

  int _x = 0, _y = 0;
  for (unsigned int i = 0; i < real.size(); ++i) {
    Color color = real[i];
    SDL_SetRenderDrawColor(gRenderer, color.r, color.g, color.b, 0);
    SDL_RenderDrawPoint(gRenderer, _x++ + x + 10, _y + y + 10);

    if (i % 100 == 0) {
      ++_y;
      _x = 0;
    }
  }

  SDL_Rect win = {x+10+c.x, y+10+c.y, 400 / c.z, 400 / c.z};
  SDL_SetRenderDrawColor(gRenderer, 0xff, 0x0, 0x0, 0);
  SDL_RenderDrawRect(gRenderer, &win);
}

class PixelArt {
 public:
    PixelArt();

    void handle_click(int, int, bool);
    void end_event();
    void undo();
    void next_color();
    void prev_color();
    void render();
    void zoom(int);
    void pan(int, int);

 private:
    std::array<Color, 100*100> real;
    TileField field;
    PaletteViewer p;
    MiniMap m;
    std::vector<std::vector<Change>> history;
};

PixelArt::PixelArt()
: field{100, 100, 400, 400, real}, p{100, 500}, m{550, 100, real} {}

void PixelArt::next_color() {
  p.next();
}

void PixelArt::prev_color() {
  p.prev();
}

void PixelArt::handle_click(int click_x, int click_y, bool shift) {
  if (click_x > 100 && click_x < 500 && click_y > 100) {
    if (click_y < 500) {
      field.handle_click(click_x, click_y, shift);
    } else if (click_y < 540) {
      p.handle_click(click_x, click_y);
      field.set_color(p.get_color());
    }
  }
}

void PixelArt::end_event() {
  field.end_event();
  if (field.changelist.size() == 0)
    return;

  history.push_back(field.changelist);
  field.changelist.clear();
}

void PixelArt::undo() {
  if (history.size() == 0)
    return;

  std::vector<Change> changelist = history.back();
  history.pop_back();
  field.play(changelist);
}

void PixelArt::render() {
  field.render();
  p.render();
  m.render(field.camera);
}

void PixelArt::zoom(int change) {
  field.zoom(change);
}

void PixelArt::pan(int change_x, int change_y) {
  field.pan(change_x, change_y);
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
    gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
        // SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
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

int main(int argc, char* args[]) {
  if (!init()) {
    printf("Failed to initialize!\n");
    return -1;
  }
  PixelArt app {};
  bool quit = false;
  bool click = false;
  bool last = false;
  bool shift = false;
  int click_x, click_y;
  while (!quit) {
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
      switch (e.type) {
        case SDL_QUIT:
          quit = true;
          break;
        case SDL_KEYDOWN:
          if (e.key.keysym.mod & KMOD_SHIFT) {
            shift = true;
          }
          switch (e.key.keysym.sym) {
            case 'q':
              quit = true;
              break;
            case SDLK_UP:
              app.zoom(1);
              break;
            case SDLK_DOWN:
              app.zoom(-1);
              break;
            case SDLK_TAB:
              if (e.key.keysym.mod & KMOD_SHIFT)
                app.prev_color();
              else
                app.next_color();
              break;
            case 'h':
              app.pan(-5, 0);
              break;
            case 'j':
              app.pan(0, 5);
              break;
            case 'k':
              app.pan(0, -5);
              break;
            case 'l' :
              app.pan(5, 0);
              break;
            case 'u':
              app.undo();
          }
          break;
        case SDL_KEYUP:
          if ((e.key.keysym.mod & KMOD_SHIFT) == 0) {
            shift = false;
          }
        case SDL_MOUSEMOTION:
          if (e.motion.state & SDL_BUTTON_LMASK) {
            click_x = e.motion.x;
            click_y = e.motion.y;
          }
          break;
        case SDL_MOUSEBUTTONDOWN:
          if (e.button.button == SDL_BUTTON_LEFT) {
            click = true;
            click_x = e.button.x;
            click_y = e.button.y;
          }
          break;
        case SDL_MOUSEBUTTONUP:
          if (e.button.button == SDL_BUTTON_LEFT)
            click = false;
          break;
      }
    }

    if (click == true) {
      app.handle_click(click_x, click_y, shift);
      last = click;
    } else if (last == true) {
      // end of sequence
      app.end_event();
    }

    // Non-white background
    SDL_SetRenderDrawColor(gRenderer, 0xBA, 0xBA, 0xBA, 0);
    SDL_RenderClear(gRenderer);

    app.render();

    SDL_RenderPresent(gRenderer);
  }

  close();

  return 0;
}
