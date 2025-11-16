// Copyright 2025 Liam Hopkins
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <random>
#include <set>
#include <string>
#include <vector>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

const int TILEWIDTH = 45;
const int TILEHEIGHT = 60;

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;

enum CardValue {
  FAKEZERO,
  ACE,
  TWO,
  THREE,
  FOUR,
  FIVE,
  SIX,
  SEVEN,
  EIGHT,
  NINE,
  TEN,
  JACK,
  QUEEN,
  KING,
  LASTVAL
};

std::map<int, std::string> ValueNames{
  {ACE, "ACE"},
  {TWO, "TWO"},
  {THREE, "THREE"},
  {FOUR, "FOUR"},
  {FIVE, "FIVE"},
  {SIX, "SIX"},
  {SEVEN, "SEVEN"},
  {EIGHT, "EIGHT"},
  {NINE, "NINE"},
  {TEN, "TEN"},
  {JACK, "JACK"},
  {QUEEN, "QUEEN"},
  {KING, "KING"}};

enum CardSuit {
  HEARTS,
  DIAMONDS,
  CLUBS,
  SPADES
};

std::map<int, std::string> SuitNames{
  {HEARTS, "HEARTS"},
  {DIAMONDS, "DIAMONDS"},
  {CLUBS, "CLUBS"},
  {SPADES, "SPADES"}};

class Card {
 public:
  Card(CardValue, CardSuit, SDL_Texture*, int, int, int, int);
  Card(const Card&);

  void Draw(SDL_Renderer*);
  void Draw(SDL_Renderer*, SDL_Rect* dst);
  bool HasIntersection(SDL_Rect* dst);
  void Move(int, int);
  CardValue value;
  CardSuit suit;

 private:
  SDL_Rect src;
  SDL_Rect dst = {100, 100, TILEWIDTH * 2, TILEHEIGHT * 2};
  SDL_Texture* texture;
};

Card::Card(CardValue v, CardSuit s, SDL_Texture* t, int x, int y, int w, int h)
  : value{v}, suit{s}, src{x, y, w, h}, texture{t} { }

Card::Card(const Card& old) : value{old.value}, suit{old.suit}, src{old.src}, texture{old.texture} {
}

std::ostream& operator<<(std::ostream& os, const Card& card) {
  return os << ValueNames[card.value] << " of " << SuitNames[card.suit];
}

std::ostream& operator<<(std::ostream& os, const SDL_Rect& rect) {
  return os << "{" << rect.x << "," << rect.y << "}";
}

std::ostream& operator<<(std::ostream& os, const SDL_Rect* rect) {
  return os << "{" << rect->x << "," << rect->y << "}";
}

void Card::Draw(SDL_Renderer* renderer) {
  SDL_RenderCopy(renderer, texture, &src, &dst);
}

void Card::Draw(SDL_Renderer* renderer, SDL_Rect* dst_) {
  SDL_RenderCopy(renderer, texture, &src, dst_);
}

void Card::Move(int x, int y) {
  std::cout << "move" << static_cast<Card>(*this) << "from {" << dst.x << "," << dst.y << "} to {" << x << "," << y << "}"
    << std::endl;
  dst.x = x;
  dst.y = y;
}

bool Card::HasIntersection(SDL_Rect* target) {
  std::cout << "hasintersection" << &dst << " and " << target << std::endl;
  return (SDL_HasIntersection(&dst, target) == SDL_TRUE);
}

bool init() {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
    return false;
  }

  /*
  if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
    printf("Warning: Linear texture filtering not enabled!");
  }
  */

  gWindow = SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED,
      SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
  if (gWindow == NULL) {
    printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
    return false;
  }
  gRenderer = SDL_CreateRenderer(gWindow, -1,
      SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (gRenderer == NULL) {
    printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
    return false;
  }

  SDL_SetRenderDrawColor(gRenderer, 0xBA, 0xBA, 0xBA, 0xBA);

  return true;
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

  auto rd = std::random_device {};
  auto rng = std::default_random_engine { rd() };


  /*
   * 52 sprites, arranged in tileset:
   *
   *     A 2 3 4 5 6 7 8 9 J Q K
   *  ♥  . . . . . . . . . . . .
   *  ⋄  . . . . . . . . . . . .
   *  ♣  . . . . . . . . . . . .
   *  ♠  . . . . . . . . . . . .
   *
   * 11px left border
   * 23px gap
   * 2px top border
   * 5px gap
   *
   */

  std::string path = "assets/Tilesheet/cardsLarge_tilemap.png";
  auto tileSurface = IMG_Load(path.c_str());
  if (!tileSurface) {
      std::cerr << "ERROR: failed to load " << path << ": "
              << IMG_GetError() << "\n";
      return 1;
  }

  auto tex = SDL_CreateTextureFromSurface(gRenderer, tileSurface);

  std::vector<Card> deck;
  int cardval = ACE;
  int cardsuit = HEARTS;
  for (int i = 0; i < 52; ++i) {
    int x = i % 13;
    int y = (i / 13);
    int realx = (x * (TILEWIDTH + 20)) + 10;
    int realy = (y * (TILEHEIGHT + 5)) + 2;
    const Card card = Card(static_cast<CardValue>(cardval), static_cast<CardSuit>(cardsuit), tex, realx,
        realy, TILEWIDTH, TILEHEIGHT);
    deck.push_back(card);
    ++cardval;
    if (cardval == LASTVAL) {
      cardval = ACE;
      ++cardsuit;
    }
  }
  std::shuffle(std::begin(deck), std::end(deck), rng);

  std::vector<Card> stacks[7];

  for (int i = 0; i < 7; ++i) {
    for (int j = 0; j < (i+1); ++j) {
      stacks[i].push_back(deck.back());
      deck.pop_back();
    }
  }

  SDL_Rect deckDst = {650, 20, TILEWIDTH * 2, TILEHEIGHT * 2};
  bool click = false;
  int idx = 0;
  SDL_Event e;
  bool quit = false;
  bool print = true;
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
            case 'h':
              --idx;
              break;
            case 'l':
              ++idx;
              break;
          }
          break;
        case SDL_MOUSEMOTION:
          if (e.motion.state & SDL_BUTTON_LMASK) {
            deckDst.x = e.motion.x;
            deckDst.y = e.motion.y;
            for (int i = 0; i < 7; ++i) {
              Card& card = stacks[i].back();
              if (card.HasIntersection(&deckDst)) {
                std::cout << "highlight card: " << card << std::endl;
              }
            }
          }
          break;
        case SDL_MOUSEBUTTONDOWN:
          if (e.button.button == SDL_BUTTON_LEFT) {
            SDL_Point p = {e.button.x, e.button.y};
            if (SDL_PointInRect(&p, &deckDst)) {
              click = true;
            }
          }
          break;
        case SDL_MOUSEBUTTONUP:
          if (e.button.button == SDL_BUTTON_LEFT && click) {
            click = false;
            deckDst.x = 650;
            deckDst.y = 20;
          }

          break;
      }
    }

    // Non-white background
    SDL_SetRenderDrawColor(gRenderer, 0xBA, 0xBA, 0xBA, 0xFF);
    SDL_RenderClear(gRenderer);

    // Draw four empty columns
    SDL_SetRenderDrawColor(gRenderer, 0x0, 0xFF, 0x0, 0xFF);
    for (int i = 0; i < 8; ++i) {
      if (i == 4 || i == 5)
        continue;
      SDL_Rect empty = { i * 90 + 20, 20, TILEWIDTH * 2, TILEHEIGHT * 2};
      SDL_RenderDrawRect(gRenderer, &empty);
    }

    // Draw foundation piles
    // TODO(.): .

    // Draw deck and discard
    deck[0].Draw(gRenderer, &deckDst);
    // TODO(.): discard pile

    // Draw tableau of stacks
    for (int i = 0; i < 7; ++i) {
      int y = 0;
      for (size_t j = 0; j < stacks[i].size(); ++j) {
        Card& card = stacks[i][j];
        if (print) {
          SDL_Rect dst = {i * 90 + 20, y * 50 + 150, TILEWIDTH * 2, TILEHEIGHT * 2};
          card.Move(dst.x, dst.y);
          std::cout << "draw " << ValueNames[card.value] << " of " << SuitNames[card.suit] << " in stack "
            << i + 1 << std::endl;
        }
        card.Draw(gRenderer);
        ++y;
      }
    }
    print = false;


    SDL_RenderPresent(gRenderer);
  }

  close();

  return 0;
}


// rules of solitaire:
// standard deck of 52 cards, shuffled
// deal 7 'stacks' of ascending height
// reserve 4 upper spaces for placing sorted cards
