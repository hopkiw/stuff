#include <vector>

struct Color {
  int r, g, b;
};

typedef std::vector<Color> Palette;

const std::vector<Palette> palettes = {
  Palette{
    Color{0xe7, 0x00, 0x59},
    Color{0x00, 0xeb, 0xdb},
    Color{0x20, 0x38, 0xef},
    Color{0x00, 0x00, 0x00},
  },
};

