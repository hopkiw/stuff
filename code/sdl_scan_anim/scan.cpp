// Copyright 2025 Liam Hopkins
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>


namespace scan_anim {

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 420;

SDL_Window* win = NULL;
SDL_Renderer* renderer = NULL;

bool Init() {
  srand(time(NULL));

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
    std::cout << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
    return false;
  }

  win = SDL_CreateWindow(
      "", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
  if (win == NULL) {
    std::cout << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
    return false;
  }

  renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (renderer == NULL) {
    std::cout << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
    return false;
  }

  if (TTF_Init() == -1) {
    std::cout << "SDL_ttf could not initialize. TTF Error: " << TTF_GetError() << std::endl;
    return false;
  }

  return true;
}

class Slider {
 public:
  explicit Slider(SDL_Rect r) : rect_{r.x, r.y, r.w, r.h} { }
  void Draw(SDL_Renderer*);
  void SetVal(float val) {
    val_ = val;
    if (val_ > rect_.w)
      val_ = rect_.w;
  }
  float GetVal() { return val_; }
  SDL_Rect GetRect() { return rect_; }

 private:
  SDL_Rect rect_;
  float val_ = 0;
};

void Slider::Draw(SDL_Renderer* renderer) {
  SDL_Rect r = rect_;
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
  SDL_RenderDrawRect(renderer, &r);

  r.w = val_;
  SDL_SetRenderDrawColor(renderer, 0, 0, 0xFF, 0xFF);
  SDL_RenderFillRect(renderer, &r);
}

}  // namespace scan_anim

typedef struct Point {
  int x;
  int y;
} Point;

std::ostream& operator<<(std::ostream& os, const Point& p) {
  return os << "{" << p.x << "," << p.y << "}";
}


int main() {
  if (!scan_anim::Init())
    return 1;

  std::vector<scan_anim::Slider> sliders;
  for (int i = 0, x = 0, y = 0; i < 56; ++i, ++x) {
    if ((i % 8) == 0 && i) {
      ++y;
      x = 0;
    }
    scan_anim::Slider slider = scan_anim::Slider({20 + (x * 51), 20 + y * 30, 50, 15});
    sliders.push_back(slider);
  }

  int active[11] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

  SDL_Rect thebox = {400, 400, 40, 40};

  Uint32 lastTicks = SDL_GetTicks();
  bool quit = false;
  bool play = false;
  while (!quit) {
    SDL_Event e;
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
            case SDLK_SPACE:
              play = true;
              break;
          }
          break;
        case SDL_MOUSEBUTTONDOWN:
          if (e.button.button == 1) {
            std::cout << "hi" << std::endl;
          }
          break;
      }
    }

    Uint32 currentTicks = SDL_GetTicks();
    float delta = currentTicks - lastTicks;
    if (delta > 50)
      std::cout << delta;

    SDL_SetRenderDrawColor(scan_anim::renderer, 0xBA, 0xBA, 0xBA, 0xFF);
    SDL_RenderClear(scan_anim::renderer);

    if (play) {
      for (int i = 0; i < 11; ++i) {
        size_t idx = active[i];
        scan_anim::Slider& s = sliders[idx];
        float val = s.GetVal();
        if (val >= 50) {
          active[i] += 5;
          val = s.GetVal();
        }
        if (active[i] > 56)
          continue;
        val += 0.12 * delta;
        s.SetVal(val);
        SDL_Rect srect = s.GetRect();
        SDL_SetRenderDrawColor(scan_anim::renderer, 0x0, 0x0, 0x0, 0xFF);
        SDL_RenderDrawLine(scan_anim::renderer, thebox.x, thebox.y, srect.x + s.GetVal(), srect.y);
      }
    }
    for (size_t i = 0; i < sliders.size(); ++i) {
      scan_anim::Slider& s = sliders[i];
      s.Draw(scan_anim::renderer);
    }

    SDL_RenderFillRect(scan_anim::renderer, &thebox);



    SDL_RenderPresent(scan_anim::renderer);
    lastTicks = currentTicks;
  }

  SDL_DestroyRenderer(scan_anim::renderer);
  SDL_DestroyWindow(scan_anim::win);

  SDL_Quit();
  return 0;
}
