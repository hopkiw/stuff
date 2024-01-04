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

const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 700;
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

struct NESHeader {
  char magic[4];
  uint8_t nRoms;
  uint8_t nVroms;
  uint8_t mapper;
  uint8_t banks;
  bool vertical = false;
  bool horizontal = false;
  bool battery = false;
  bool trainer = false;
  bool quad = false;
  bool ntsc = false;
  bool pal = false;
};

std::ostream& operator<<(std::ostream& os, const NESHeader& hdr) {
  os << "NESHeader: " << (int)hdr.nRoms << "x16kB ROM " << (int)hdr.nVroms << "x8kB VROM";
  if (hdr.vertical)
    os << " [V-mirror]";
  if (hdr.horizontal)
    os << " [H-mirror]";
  if (hdr.battery)
    os << " w/ Battery VRAM";
  if (hdr.ntsc)
    os << " NTSC";
  if (hdr.pal)
    os << " PAL";
  os << " using mapper 0x" << std::hex << (int)hdr.mapper;
  return os;
}

NESHeader read_rom(std::ifstream& fh) {
  NESHeader hdr;
  fh.read(hdr.magic, 4);
  if (hdr.magic[0] != 'N')
    throw "bad magic";
  if (hdr.magic[1] != 'E')
    throw "bad magic";
  if (hdr.magic[2] != 'S')
    throw "bad magic";
  if (hdr.magic[3] != '')
    throw "bad magic";

  fh.read(reinterpret_cast<char *>(&hdr.nRoms), 1);
  fh.read(reinterpret_cast<char *>(&hdr.nVroms), 1);

  char buf;
  fh.read(&buf, 1);
  if (buf & 1)
    hdr.vertical = true;
  else
    hdr.horizontal = true;

  if (buf & (1 << 1))
    hdr.vertical = true;

  if (buf & (1 << 2))
    hdr.vertical = true;

  if (buf & (1 << 3))
    hdr.vertical = true;

  if (buf & (1 << 4))
    hdr.vertical = true;

  hdr.mapper = (buf & 0xf0) >> 4;

  fh.read(&buf, 1);
  hdr.mapper = hdr.mapper | (buf >> 4);

  fh.read(reinterpret_cast<char *>(&hdr.banks), 1);
  if (hdr.banks == 0)
    hdr.banks = 1;

  fh.read(&buf, 1);
  if (buf & 1)
    hdr.pal = true;
  else
    hdr.ntsc = true;

  return hdr;
}

char data[0x2000];

// typedef char PatternTile[16];

void load_tile(int idx, char* tile) {
  idx = idx << 4;
  idx = idx | 0x1000;
  for (int i = 0; i < 16; ++i) {
    tile[i] = data[idx + i];
  }
}

struct Color {
  int r, g, b;
};

Color colors[4] = {
  Color{0x00, 0x00, 0x00},
  Color{0x00, 0xeb, 0xdb},
  Color{0xff, 0xe7, 0xa2},
  Color{0x20, 0x38, 0xef},
};

void draw_tile(int x, int y, int idx) {
  char tile[16] = {};
  Color color;
  load_tile(idx, tile);
  for (int i = 0; i < 8; ++i) {
    uint8_t plane_a = tile[i];
    uint8_t plane_b = tile[i + 8];
    for (int j = 0; j < 8; ++j) {
      int mask = 1 << (7 - j);
      if ((plane_a & mask) && (plane_b & mask)) {
        color = colors[3];
      } else if (plane_a & mask) {
        color = colors[1];
      } else if (plane_b & mask) {
        color = colors[2];
      } else {
        color = colors[0];  // black
      }

      SDL_Rect t {x + (j * 5), y + (i * 5), 5, 5};
      SDL_SetRenderDrawColor(gRenderer, color.r, color.g, color.b, 0);
      SDL_RenderFillRect(gRenderer, &t);
    }
  }
}

int main(int argc, char* args[]) {
  if (!init()) {
    printf("Failed to initialize!\n");
    return -1;
  }

  std::ifstream fh;
  fh.open("Donkey.nes", std::fstream::binary);
  if (fh.fail()) {
    std::cout << "Error: Donkey.nes " << strerror(errno) << std::endl;
    return 1;
  }

  NESHeader hdr = read_rom(fh);
  std::cout << hdr << std::endl;

  unsigned int chr_start = 0x10 + (hdr.nRoms * 0x10 * 0x400);
  std::cout << "start at: 0x" << chr_start << std::endl;
  fh.seekg(0);
  fh.seekg(chr_start);
  fh.read(data, 0x2000);
  // fh.read(data, 0xff0);  // one table

  bool quit = false;

  SDL_Event e;

  int click_x = 0, click_y = 0;
  // bool click = false;

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
          }
          break;
        case SDL_MOUSEMOTION:
          if (e.motion.state & SDL_BUTTON_LMASK) {
            click_x = e.motion.x;
            click_y = e.motion.y;
          }
          break;
        case SDL_MOUSEBUTTONDOWN:
          if (e.button.button == SDL_BUTTON_LEFT) {
            click_x = e.button.x;
            click_y = e.button.y;
            SDL_Log("clicked (%d,%d)", click_x, click_y);
          }
          break;
        case SDL_MOUSEBUTTONUP:
          if (e.button.button == SDL_BUTTON_LEFT)
          break;
      }
    }

    // Non-white background
    // SDL_SetRenderDrawColor(gRenderer, 0xBA, 0xBA, 0xBA, 0xBA);
    SDL_RenderClear(gRenderer);

    int tiles[22] = {
      0xD3,
      0x01,
      0x09,
      0x08,
      0x01,
      0x24,
      0x17,
      0x12,
      0x17,
      0x1D,
      0x0E,
      0x17,
      0x0D,
      0x18,
      0x24,
      0x0C,
      0x18,
      0x65,
      0x15,
      0x1D,
      0x0D,
      0x64
    };

    int x = 0, y = 0;
    for (int i = 0; i < 22; ++i) {
      draw_tile((x * 40) + 20, (y * 40) + 250, tiles[i]);
      ++x;
    }
    /*
    for (int i = 0; i < 0x100; ++i) {
      draw_tile((x * 40) + 5, (y * 40) + 5, i);
      ++x;
      if ((i+1) % 16 == 0) {
        x = 0;
        ++y;
      }
    }
    */

    SDL_RenderPresent(gRenderer);
  }

  close();

  return 0;
}
