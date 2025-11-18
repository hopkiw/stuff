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
  JOKER,
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
  {KING, "KING"},
  {JOKER, "JOKER"},
};

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
  // Card(const Card&);

  void Draw(SDL_Renderer*);
  // void Draw(SDL_Renderer*, SDL_Rect* dst);
  bool HasIntersection(SDL_Rect* dst);
  bool HasIntersection(const Card&);
  void Move(int, int);
  void RelMove(int, int);

  CardValue value;
  CardSuit suit;

 private:
  SDL_Rect src;
  SDL_Rect dst;
  SDL_Texture* texture;
};

std::ostream& operator<<(std::ostream& os, const Card& card) {
  return os << "<" << ValueNames[card.value] << " of " << SuitNames[card.suit] << ">";
}

std::ostream& operator<<(std::ostream& os, const SDL_Rect& rect) {
  return os << "SDL_Rect{" << rect.x << "," << rect.y << "+" << rect.w << "+" << rect.h << "}";
}

std::ostream& operator<<(std::ostream& os, const SDL_Rect* rect) {
  return os << "SDL_Rect{" << rect->x << "," << rect->y << "+" << rect->w << "+" << rect->h << "}";
}

Card::Card(CardValue v, CardSuit s, SDL_Texture* t, int x, int y, int w, int h)
  : value{v}, suit{s}, src{x, y, w, h}, texture{t} { }

void Card::Draw(SDL_Renderer* renderer) {
  SDL_RenderCopy(renderer, texture, &src, &dst);
}

/*
void Card::Draw(SDL_Renderer* renderer, SDL_Rect* dst_) {
  SDL_RenderCopy(renderer, texture, &src, dst_);
}
*/

bool Card::HasIntersection(SDL_Rect* target) {
  return (SDL_HasIntersection(&dst, target) == SDL_TRUE);
}

bool Card::HasIntersection(const Card& card) {
  return (SDL_HasIntersection(&dst, &card.dst) == SDL_TRUE);
}

void Card::Move(int x, int y) {
  dst.x = x;
  dst.y = y;
  dst.w = 2 * TILEWIDTH;
  dst.h = 2 * TILEHEIGHT;
}

void Card::RelMove(int x, int y) {
  dst.x += x;
  dst.y += y;
}

bool init() {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cout << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
    return false;
  }

  gWindow = SDL_CreateWindow(
      "", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
  if (gWindow == NULL) {
    std::cout << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
    return false;
  }
  gRenderer = SDL_CreateRenderer(gWindow, -1,
      SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (gRenderer == NULL) {
    std::cout << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
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

bool isStackValid(const Card* first, const Card* second) {
  bool validSuit = true, validValue = true;

  if (first->suit == second->suit)
    validSuit = false;
  if (first->suit == HEARTS && second->suit == DIAMONDS)
    validSuit = false;
  if (first->suit == DIAMONDS && second->suit == HEARTS)
    validSuit = false;
  if (first->suit == CLUBS && second->suit == SPADES)
    validSuit = false;
  if (first->suit == SPADES && second->suit == CLUBS)
    validSuit = false;

  if (second->value - first->value != 1 && first->value - second->value != 1)
    validValue = false;

  return validSuit && validValue;
}

bool isFoundationValid(const Card* first, const Card* second) {
  std::cout << "check if " << *first << "can go on foundation card " << *second << std::endl;
  return (first->suit == second->suit) && (first->value == (second->value - 1));
}

int main() {
  if (!init()) {
    std::cout << "Failed to initialize!" << std::endl;
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
      std::cerr << "ERROR: failed to load " << path << ": " << IMG_GetError() << std::endl;
      return 1;
  }

  auto tex = SDL_CreateTextureFromSurface(gRenderer, tileSurface);
  if (!tex) {
      std::cerr << "ERROR: failed to create texture: " << IMG_GetError() << std::endl;
      return 1;
  }

  std::vector<Card> allcards;
  int cardval = ACE;
  int cardsuit = HEARTS;
  for (int i = 0; i < 56; ++i) {
    int x = i % 14;
    int y = (i / 14);

    int realx = (x * (TILEWIDTH + 20)) + 10;
    int realy = (y * (TILEHEIGHT + 5)) + 2;

    const Card card = {
      static_cast<CardValue>(cardval),
      static_cast<CardSuit>(cardsuit),
      tex,
      realx,
      realy,
      TILEWIDTH,
      TILEHEIGHT
    };

    allcards.push_back(card);

    ++cardval;
    if (cardval == LASTVAL) {
      cardval = ACE;
      ++cardsuit;
    }
  }

  std::vector<Card> deck;
  std::vector<Card> stacks[7];
  std::vector<Card> foundations[4];
  std::vector<Card> draggedCards;

  for (int i = 0; i < 52; ++i) {
    if (((i+1) % 14) == 0)
      ++i;
    deck.push_back(allcards[i]);
  }

  std::shuffle(std::begin(deck), std::end(deck), rng);

  for (int i = 0; i < 7; ++i) {
    for (int j = 0; j < (i+1); ++j) {
      stacks[i].push_back(deck.back());
      deck.pop_back();
    }
  }

  SDL_Rect deckRect = {650, 20, TILEWIDTH * 2, TILEHEIGHT * 2};
  // SDL_Rect discardRect = {630, 20, TILEWIDTH * 2, TILEHEIGHT * 2};;
  SDL_Rect foundationRects[4];
  SDL_Rect stackRects[7];

  for (int i = 0; i < 4; ++i) {
    foundationRects[i] = { i * 90 + 20, 20, TILEWIDTH * 2, TILEHEIGHT * 2};
  }

  for (int i = 0; i < 7; ++i) {
    stackRects[i] = { i * 90 + 20, 150, TILEWIDTH * 2, TILEHEIGHT * 2};
  }

  SDL_Event e;
  bool quit = false;
  int idx = 0;
  int srcStack = -1;

//  int count = 0;
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
            for (auto it = draggedCards.begin(); it != draggedCards.end(); ++it) {
              it->RelMove(e.motion.xrel, e.motion.yrel);
            }
          }
          break;
        case SDL_MOUSEBUTTONDOWN:
          if (e.button.button == SDL_BUTTON_LEFT) {
            /*
            auto ticks = SDL_GetTicks();
            if ((ticks - lastClicked) < 300)
              std::cout << "double-click!" << std::endl;
            lastClicked = ticks;
            */

            SDL_Rect r = {e.button.x, e.button.y, 1, 1};

            for (int i = 0; i < 7; ++i) {
              for (auto rit = stacks[i].rbegin(); rit != stacks[i].rend(); ++rit) {
                if (rit != stacks[i].rbegin() && !isStackValid(&(*rit), &(*(rit - 1)))) {
                  break;
                }

                if (rit->HasIntersection(&r)) {
                  draggedCards = std::vector<Card>(std::next(rit).base(), stacks[i].end());
                  stacks[i].erase(std::next(rit).base(), stacks[i].end());
                  srcStack = i;
                  std::cout << "clicked on card(s) ";
                  for (auto ccard : draggedCards)
                    std::cout << ccard;
                  std::cout << std::endl;
                  goto label;
                }
              }
            }
          }
label:
          break;
        case SDL_MOUSEBUTTONUP:
          if (e.button.button == SDL_BUTTON_LEFT) {
            if (draggedCards.size()) {
              if (draggedCards.size() == 1) {
                // check foundations
                for (int i = 0; i < 4; ++i) {
                  SDL_Rect empty = { i * 90 + 20, 20, TILEWIDTH * 2, TILEHEIGHT * 2};
                  if (draggedCards.size() > 0 && draggedCards.back().HasIntersection(&empty)) {
                    if (foundations[i].empty() && draggedCards.back().value == ACE) {
                      std::cout << "ok, move card " << draggedCards.back() << " to empty foundation " << i
                        << std::endl;
                      foundations[i].insert(foundations[i].end(), draggedCards.back());
                      draggedCards.clear();
                      srcStack = -1;
                      break;
                    }
                    if (foundations[i].size() && isFoundationValid(&foundations[i].back(), &draggedCards.back())) {
                      std::cout << "ok, move card " << draggedCards.back() << " onto foundation " << i
                        << std::endl;
                      foundations[i].insert(foundations[i].end(), draggedCards.back());
                      draggedCards.clear();
                      srcStack = -1;
                      break;
                    }
                  }
                }
              }
              auto dragcard = draggedCards.begin();
              for (int i = 0; i < 7; ++i) {
                if (stacks[i].size()) {
                  auto card = stacks[i].back();
                  if (dragcard->HasIntersection(card) && isStackValid(&card, &*dragcard)) {
                    srcStack = i;
                    break;
                  }
                } else {
                  if (dragcard->HasIntersection(&stackRects[i]) && dragcard->value == KING) {
                    srcStack = i;
                    break;
                  }
                }
              }
              if (srcStack >= 0) {
                stacks[srcStack].insert(stacks[srcStack].end(), draggedCards.begin(), draggedCards.end());
                draggedCards.clear();
                srcStack = -1;
              }
            }
          }
          break;
      }
    }

    // Non-white background
    SDL_SetRenderDrawColor(gRenderer, 0xBA, 0xBA, 0xBA, 0xFF);
    SDL_RenderClear(gRenderer);

    // Draw foundation piles
    SDL_SetRenderDrawColor(gRenderer, 0x0, 0xFF, 0x0, 0xFF);
    for (int i = 0; i < 4; ++i) {
      SDL_RenderDrawRect(gRenderer, &foundationRects[i]);
      if (foundations[i].empty())
        continue;
      auto rit = foundations[i].rbegin();
      rit->Move(foundationRects[i].x, foundationRects[i].y);
      rit->Draw(gRenderer);
    }

    // Draw deck
    SDL_RenderDrawRect(gRenderer, &deckRect);
    if (deck.size()) {
      Card top = deck[0];
      top.Move(deckRect.x, deckRect.y);
      top.Draw(gRenderer);
    }

    // Draw tableau of stacks
    for (int i = 0; i < 7; ++i) {
      int y = 0;
      for (auto it = stacks[i].begin(); it != stacks[i].end(); ++it, ++y) {
        it->Move(stackRects[i].x, stackRects[i].y + (y * 35));
        it->Draw(gRenderer);
      }
    }

    // Draw card(s) being dragged
    for (auto it = draggedCards.begin(); it != draggedCards.end(); ++it) {
      it->Draw(gRenderer);
    }

    SDL_RenderPresent(gRenderer);
  }

  close();

  return 0;
}
