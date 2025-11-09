// Copyright 2025 gwriterk
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

const int SCREEN_WIDTH = 750;
const int SCREEN_HEIGHT = 600;

bool init();
bool loadMedia();
void close();

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
TTF_Font* gFont = NULL;

class Block {
 public:
  Block();
  Block(int, int, SDL_Color);

  bool Draw();

  SDL_Rect rect;
  SDL_Color color;
  float vel_x = 0.0f;
  float vel_y = 0.0f;
};

SDL_Color colors[] = {
  SDL_Color{0xFF, 0x0, 0x0},
  SDL_Color{0x0, 0xFF, 0x0},
  SDL_Color{0x0, 0x0, 0xFF},
};

Block::Block() :
  rect{SCREEN_WIDTH, static_cast<int>(lrand48() % SCREEN_HEIGHT), TILEWIDTH, TILEWIDTH},
  color{colors[lrand48() % 3]},
  vel_x{((lrand48() % 4) + 5) / 100.0f} { }

Block::Block(int x, int y, SDL_Color color_) :
  rect{x, y, TILEWIDTH, TILEWIDTH},
  color{color_} { }

bool Block::Draw() {
  int rc = SDL_RenderDrawRect(gRenderer, &rect);
  if (rc != 0) {
      std::cerr << "SDL_RenderDrawRect failed: " << SDL_GetError() << std::endl;
      return false;
  }
  SDL_SetRenderDrawColor(gRenderer, 0xFF, 0x0, 0x0, 0xFF);
  rc = SDL_RenderFillRect(gRenderer, &rect);
  if (rc != 0) {
      std::cerr << "SDL_RenderFillRect failed: " << SDL_GetError() << std::endl;
      return false;
  }

  return true;
}

class Slider {
 public:
  Slider(int, int, int);

  bool Draw();
  void SetFill(int);
  int GetFill();
  void SetClicked(bool);
  bool GetClicked();

  SDL_Rect rect;

 private:
  int fill;
  bool clicked = false;
};

Slider::Slider(int x, int y, int fill_) : rect{x, y, 190, 20}, fill{fill_} {
  std::cout << "new slider" << std::endl;
}

bool Slider::Draw() {
      // slider form
      SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0xFF);  // black
      SDL_RenderDrawRect(gRenderer, &rect);

      // slider fillbar
      // int pixels = fill * rect.w;
      SDL_SetRenderDrawColor(gRenderer, 0, 0xFF, 0, 0xFF);  // green
      SDL_Rect fillRect = {rect.x, rect.y, (fill * rect.w) / 100, rect.h};
      SDL_RenderFillRect(gRenderer, &fillRect);

      // slider text
      SDL_Color textColor = {0, 0, 0, 0xFF};
      SDL_Surface* mySurface = TTF_RenderText_Solid(gFont, std::to_string(fill).c_str(), textColor);
      if (mySurface == NULL) {
        return false;
      }

      SDL_Texture* myTexture = SDL_CreateTextureFromSurface(gRenderer, mySurface);

      SDL_Rect textRect = {
        rect.x + rect.w + 10,
        rect.y,
        mySurface->w,
        mySurface->h,
      };

      SDL_RenderCopy(gRenderer, myTexture, NULL, &textRect);

      return true;
}

void Slider::SetFill(int fill_) {
  fill = fill_;

  if (fill > 100)
    fill = 100;

  if (fill < 0)
    fill = 0;

  std::cout << "set fill to " << fill << std::endl;
}

int Slider::GetFill() {
  return fill;
}

void Slider::SetClicked(bool clicked_) {
  std::cout << "clicked: " << clicked_ << std::endl;
  clicked = clicked_;
}

bool Slider::GetClicked() {
  return clicked;
}

bool init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    gWindow = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (gWindow == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (gRenderer == NULL) {
        printf("Renderer could not be created. SDL Error: %s\n", IMG_GetError());
        return false;
    }

    if (TTF_Init() == -1) {
      printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
      return false;
    }

    gFont = TTF_OpenFont("/usr/share/fonts/truetype/terminus/TerminusTTF-4.46.0.ttf", 20);
    if (gFont == NULL) {
        printf("Font could not be created. SDL Error: %s\n", TTF_GetError());
        return false;
    }

    return true;
}

void close() {
    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    gWindow = NULL;
    gRenderer = NULL;

    IMG_Quit();
    SDL_Quit();
}

int main(int argc, char* args[]) {
    Uint16 stime = time(NULL);
    seed48(&stime);

    if (!init()) {
      printf("Failed to initialize!\n");
      return 1;
    }

    Slider speedSlider = {20, 20, 0};
    Slider createChanceSlider = {20, 50, 0};

    // std::vector<Slider*> sliders = {&speedSlider, &createChanceSlider};
    std::vector<Slider*> sliders = {&speedSlider};

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
            return 0;
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
                  s->SetClicked(true);
                }
              }
            }
            break;
          case SDL_MOUSEBUTTONUP:
            for (auto s : sliders) {
              if (s->GetClicked()) {
                s->SetClicked(false);
              }
            }
            break;
          case SDL_MOUSEMOTION:
            for (auto s : sliders) {
              if (s->GetClicked() == true) {
                std::cout << "adding to fill" << std::endl;
                s->SetFill(s->GetFill() + e.motion.xrel);
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
      if ((chance > 75) && ((currentTicks - lastCreatedTime) > 200) && (blocks.size() < 3)) {
        Block block;
        blocks.push_back(block);
        lastCreatedTime = currentTicks;
        std::cout << "new block" << std::endl;
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

      if (deleted)
        std::cout << "deleted " << deleted << " blocks, now there are " << blocks.size() << std::endl;

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
      SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
      SDL_RenderClear(gRenderer);

      for (auto block : blocks) {
        block.Draw();
      }

      for (auto s : sliders) {
        s->Draw();
      }

      cat.Draw();

      SDL_RenderPresent(gRenderer);
      lastTicks = currentTicks;
    }
    close();

    return 0;
}

