#include <SDL2/SDL.h>
#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <gif_lib.h>

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <set>
#include <array>
#include <vector>

using namespace std;

struct Color {
  int r, g, b;
  Color(int _r, int _g, int _b) : r{_r}, g{_g}, b{_b} {}
};

/*
function rgb2hue(r, g, b) {
  r /= 255;
  g /= 255;
  b /= 255;
  var max = Math.max(r, g, b);
  var min = Math.min(r, g, b);
  var c   = max - min;
  var hue;
  if (c == 0) {
    hue = 0;
  } else {
    switch(max) {
      case r:
        var segment = (g - b) / c;
        var shift   = 0 / 60;       // R° / (360° / hex sides)
        if (segment < 0) {          // hue > 180, full rotation
          shift = 360 / 60;         // R° / (360° / hex sides)
        }
        hue = segment + shift;
        break;
      case g:
        var segment = (b - r) / c;
        var shift   = 120 / 60;     // G° / (360° / hex sides)
        hue = segment + shift;
        break;
      case b:
        var segment = (r - g) / c;
        var shift   = 240 / 60;     // B° / (360° / hex sides)
        hue = segment + shift;
        break;
    }
  }
  return hue * 60; // hue is in [0,6], scale it up
}
*/

int rgb2hue(int r, int g, int b) {
  float _r = r / 255.0;
  float _g = g / 255.0;
  float _b = b / 255.0;

  float max = _r;
  char s = 'r';
  if (_g > max) {
    s = 'g';
    max = _g;
  }
  if (b > max) {
    s = 'b';
    max = _b;
  }

  float min = r;
  if (min > _g)
    min = _g;
  if (min > _b)
    min = _b;

  float c = max - min;

  if (c == 0)
    return 0;

  float hue;
  float segment;
  float shift;
  switch (s) {
    case 'r':
      segment = (_g - _b) / c;
      shift = 0 / 60.0;
      if (segment < 0) {
        shift = 6;
      }
      hue = segment + shift;
      break;
    case 'g':
      segment = (_b - _r) / c;
      shift = 2;
      hue = segment + shift;
      break;
    case 'b':
      segment = (_r - _g) / c;
      shift = 4;
      hue = segment + shift;
      break;
  }
  int res = (hue * 60) + 0.5;
  return res;
}

int main() {
  Color color {22, 38, 255};
  int hsl = rgb2hue(color.r, color.g, color.b);
  printf("convert %d,%d,%d to hsl: %d\n", color.r, color.g, color.b, hsl);
  return 0;
}
