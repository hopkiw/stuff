#include <string.h>

#include <iostream>
#include <iomanip>
#include <fstream>

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

int main() {
  std::ifstream fh;
  fh.open("Donkey.nes", std::fstream::binary);
  // fh.open("Donkey.nes", std::fstream::binary);
  if (fh.fail()) {
    std::cout << "Error: Donkey.nes " << strerror(errno) << std::endl;
    return 1;
  }

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

  std::cout << hdr << std::endl;

  unsigned int chr_start = 0x10 + (hdr.nRoms * 0x10 * 0x400);
  unsigned int table = 0;
  char chr = 0x0;
  unsigned int target = ((chr + (table * 0x100)) * 0x10);
  target = target + chr_start;
  std::cout << "VROM starts at byte " << std::hex << chr_start << std::endl;
  std::cout << "selecting chr 0x2D at addr " << std::hex << target << std::endl;

  fh.seekg(0);
  fh.seekg(target);
  char tile[16];
  fh.read(tile, 16);
  for (int i = 0; i < 8; ++i) {
    uint8_t plane_a = tile[i];
    uint8_t plane_b = tile[i + 8];
    std::cout << "plane a is " << (int)plane_a;
    std::cout << std::endl;
    std::cout << "plane b is " << (int)plane_b;
    std::cout << std::endl;
    for (int j = 7; j >= 0; --j) {
      if ((plane_a & (1 << j)) && (plane_b & (1 << j)))
        std::cout << '3';
      else if (plane_a & (1 << j))
        std::cout << '1';
      else if (plane_b & (1 << j))
        std::cout << '2';
      else
        std::cout << '.';
    }
    std::cout << std::endl;
  }


  return 0;
}
