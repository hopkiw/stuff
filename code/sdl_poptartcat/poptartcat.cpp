// Copyright 2025 hopkiw

#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <unistd.h>
#include <stdlib.h>

#include <ctime>
#include <string>
#include <iostream>
#include <cmath>
#include <vector>

#define TILEWIDTH 75

namespace poptartcat {

const int SCREEN_WIDTH = 750;
const int SCREEN_HEIGHT = 600;

class Block {
 public:
  explicit Block(float);
  Block(int, int, SDL_Color);

  bool Draw(SDL_Renderer*);

  SDL_Rect rect;
  SDL_Color color;
  float vel_x = 0.0f;
  float vel_y = 0.0f;
};

SDL_Color colors[] = {
  SDL_Color{0xFF, 0x0, 0x0, 0xFF},
  SDL_Color{0x0, 0xFF, 0x0, 0xFF},
  SDL_Color{0x0, 0x0, 0xFF, 0xFF},
};

Block::Block(float vel) :
  rect{SCREEN_WIDTH, static_cast<int>(lrand48() % SCREEN_HEIGHT), TILEWIDTH, TILEWIDTH},
  color{colors[lrand48() % 3]},
  vel_x{vel} { }

Block::Block(int x, int y, SDL_Color color_) :
  rect{x, y, TILEWIDTH, TILEWIDTH},
  color{color_} { }

bool Block::Draw(SDL_Renderer* renderer) {
  int rc = SDL_RenderDrawRect(renderer, &rect);
  if (rc != 0) {
      std::cerr << "SDL_RenderDrawRect failed: " << SDL_GetError() << std::endl;
      return false;
  }
  SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
  rc = SDL_RenderFillRect(renderer, &rect);
  if (rc != 0) {
      std::cerr << "SDL_RenderFillRect failed: " << SDL_GetError() << std::endl;
      return false;
  }

  return true;
}

class Slider {
 public:
  Slider(std::string, TTF_Font*, int, int, int, int, int);

  bool Draw(SDL_Renderer*);
  void SetVal(int);
  unsigned int GetVal();
  void SetClicked(int);
  bool GetClicked();
  void SetUnclicked();

  SDL_Rect rect;

 private:
  TTF_Font* font;
  std::string label;
  int val;
  int max = 100;
  int min = 0;
  bool clicked = false;
};

Slider::Slider(std::string label_, TTF_Font* font_, int x, int y, int val_, int min_, int max_) :
  rect{x, y, 190, 20},
  font{font_},
  label{label_},
  val{val_},
  max{max_},
  min{min_} {
}

bool Slider::Draw(SDL_Renderer* renderer) {
      // outline
      SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);  // black
      SDL_RenderDrawRect(renderer, &rect);

      // fillbar
      SDL_SetRenderDrawColor(renderer, 0, 0xFF, 0, 0xFF);  // green

      float pct = static_cast<float>(val - min) / static_cast<float>(max - min);
      int pixels = pct * rect.w;

      SDL_Rect fillRect = {rect.x, rect.y, pixels, rect.h};
      SDL_RenderFillRect(renderer, &fillRect);

      // val text
      SDL_Color textColor = {0, 0, 0, 0xFF};
      SDL_Surface* mySurface = TTF_RenderText_Solid(font, std::to_string(val).c_str(), textColor);
      if (mySurface == NULL) {
        return false;
      }

      SDL_Texture* myTexture = SDL_CreateTextureFromSurface(renderer, mySurface);

      SDL_Rect textRect = {
        rect.x + rect.w + 10,
        rect.y,
        mySurface->w,
        mySurface->h,
      };

      SDL_RenderCopy(renderer, myTexture, NULL, &textRect);

      // label text
      mySurface = TTF_RenderText_Solid(font, label.c_str(), textColor);
      if (mySurface == NULL) {
        return false;
      }

      myTexture = SDL_CreateTextureFromSurface(renderer, mySurface);

      textRect = {
        rect.x + rect.w + 40,
        rect.y,
        mySurface->w,
        mySurface->h,
      };

      SDL_RenderCopy(renderer, myTexture, NULL, &textRect);

      return true;
}

void Slider::SetVal(int val_) {
  val = val_;

  if (val > max)
    val = max;

  if (val < min)
    val = min;
}

unsigned int Slider::GetVal() {
  return val;
}

void Slider::SetClicked(int x) {
  clicked = true;
  float offset = x - rect.x;
  float ratio = offset / rect.w;
  SetVal((ratio * (max - min)) + min);
}

bool Slider::GetClicked() {
  return clicked;
}

void Slider::SetUnclicked() {
  clicked = false;
}

class Poptartcat {
 public:
  Poptartcat() {}
  void Run();

 private:
  SDL_Window* window = NULL;
  SDL_Renderer* renderer = NULL;
  TTF_Font* font = NULL;
  bool init();
  void close();
};

bool Poptartcat::init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        printf("Renderer could not be created. SDL Error: %s\n", IMG_GetError());
        return false;
    }

    if (TTF_Init() == -1) {
      printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
      return false;
    }

    font = TTF_OpenFont("/usr/share/fonts/truetype/terminus/TerminusTTF-4.46.0.ttf", 20);
    if (font == NULL) {
        printf("Font could not be created. SDL Error: %s\n", TTF_GetError());
        return false;
    }

    return true;
}

void Poptartcat::close() {
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  window = NULL;
  renderer = NULL;

  IMG_Quit();
  SDL_Quit();
}

void Poptartcat::Run() {
  Uint16 stime = time(NULL);
  seed48(&stime);

  using poptartcat::Block;
  using poptartcat::Slider;
  using poptartcat::SCREEN_WIDTH;
  using poptartcat::SCREEN_HEIGHT;

  if (!init()) {
    return;
  }

  Slider speedSlider = {"block speed", font, 20, 20, 4, 1, 13};
  Slider createChanceSlider = {"create rate", font, 20, 50, 75, 75, 99};
  Slider maxBlocksSlider = {"max blocks", font, 20, 80, 5, 0, 100};
  Slider timeBetweenBlocksSlider = {"time between blocks", font, 20, 110, 200, 0, 800};

  std::vector<Slider*> sliders = {&speedSlider, &createChanceSlider, &maxBlocksSlider,
    &timeBetweenBlocksSlider};

  int cat_original_y = SCREEN_HEIGHT / 2;
  Block cat = {SCREEN_WIDTH / 2, cat_original_y, SDL_Color{0x3, 0xFC, 0x7F, 0xFF}};

  std::vector<Block> blocks;
  Uint32 lastTicks = SDL_GetTicks();
  Uint32 lastCreatedTime = lastTicks;

  while (true) {
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
      switch (e.type) {
        case SDL_QUIT:
          return;
          break;
        case SDL_KEYDOWN:
          if (e.key.keysym.sym == SDLK_SPACE) {
            if (cat.rect.y == cat_original_y) {
              cat.vel_y = -0.7;
            }
          }
          break;
        case SDL_MOUSEBUTTONDOWN:
          if (e.button.button == 1) {
            SDL_Point p = {e.button.x, e.button.y};
            for (auto s : sliders) {
              if (SDL_PointInRect(&p, &s->rect)) {
                s->SetClicked(p.x);
              }
            }
          }
          break;
        case SDL_MOUSEBUTTONUP:
          for (auto s : sliders) {
            if (s->GetClicked()) {
              s->SetUnclicked();
            }
          }
          break;
        case SDL_MOUSEMOTION:
          for (auto s : sliders) {
            if (s->GetClicked() == true) {
              s->SetClicked(e.motion.x);
            }
          }
          break;
        default:
          break;
      }
    }

    Uint32 currentTicks = SDL_GetTicks();
    float delta = (currentTicks - lastTicks);

    int chance = lrand48() % 100;
    if (
        (chance > 75) &&
        ((currentTicks - lastCreatedTime) > 200) &&
        (blocks.size() < maxBlocksSlider.GetVal())) {
      float speed = speedSlider.GetVal() / 10.0f;
      Block block {speed};
      blocks.push_back(block);
      lastCreatedTime = currentTicks;
    }

    int deleted = 0;
    for (auto it = blocks.begin(); it != blocks.end();) {
      int diff = it->vel_x * delta;
      if (diff > 0)
        it->rect.x -= diff;

      if (it->rect.x < (0 - TILEWIDTH - 5)) {
        it = blocks.erase(it);
        deleted++;
      } else {
        it++;
      }
    }

    // cat motion
    if (cat.vel_y) {
      int catdiff = cat.vel_y * delta;
      cat.rect.y += catdiff;
      cat.vel_y += 0.1;
      if (cat.rect.y > cat_original_y) {
        cat.vel_y = 0;
        cat.rect.y = cat_original_y;
      }
    }

    // clear screen
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(renderer);

    for (auto block : blocks) {
      block.Draw(renderer);
    }

    for (auto s : sliders) {
      s->Draw(renderer);
    }

    cat.Draw(renderer);

    SDL_RenderPresent(renderer);
    lastTicks = currentTicks;
  }

  close();

  return;
}

}  // namespace poptartcat

int main() {
  poptartcat::Poptartcat pc;
  pc.Run();
  return 0;
}
