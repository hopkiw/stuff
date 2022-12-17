#!/usr/bin/env python3

import sys


def read_input_file(filename):
  lines = []
  with open(filename) as thefile:
    for line in thefile:
      lines.append(line.rstrip())

  return lines


def get_dimensions(lines):
  x = y = max_x = max_y = min_x = min_y = 0

  for line in lines:
    direction, length = line.split()
    length = int(length)
    if direction == 'R':
        x += length
        if x > max_x:
            max_x = x

    elif direction == 'L':
        x -= length
        if x < min_x:
            min_x = x

    elif direction == 'U':
        y += length
        if y > max_y:
            max_y = y

    elif direction == 'D':
        y -= length
        if y < min_y:
            min_y = y

  return min_x, min_y, max_x, max_y


class Point:
  x = 0
  y = 0

  def __eq__(self, value, /):
    return self.x == value.x and self.y == value.y

  def __repr__(self):
    return str((self.x, self.y))


class Board:
  def __init__(self):
    self.knots = []
    # for i in range(10):
    for i in range(2):
      self.knots.append(Point())

    self.ts = set()
    self.min_x = 0
    self.min_y = 0
    self.max_x = 0
    self.max_y = 0

  def draw(self):
    print()
    print('--Board--')

    pad = len(self.knots) + 1
    for y in range(self.min_y - pad, self.max_y + pad):
      row = ''
      for x in range(self.min_x - pad, self.max_x + pad):
        char = 'x'
        if (x, y) in self.ts:
          char = 'X'

        for knot in self.knots[1:]:
          if x == knot.x and y == knot.y:
            char = 'T'
            break

        if x == self.knots[0].x and y == self.knots[0].y:
          char = 'H'

        if x == 0 and y == 0:
          char = '0'

        row += char
      print(row)

    print('--End Board--')
    print()
    input()

  def move_head(self, d):
    if d == 'R':
      self.knots[0].x += 1

      if self.knots[0].x > self.max_x:
          self.max_x = self.knots[0].x

    elif d == 'L':
      self.knots[0].x -= 1

      if self.knots[0].x < self.min_x:
        self.min_x = self.knots[0].x

    elif d == 'U':
      self.knots[0].y += 1

      if self.knots[0].y > self.max_y:
          self.max_y = self.knots[0].y

    elif d == 'D':
      self.knots[0].y -= 1

      if self.knots[0].y < self.min_y:
        self.min_y = self.knots[0].y

  def move_tail(self, idx):
    if idx == 0:
      raise Exception('knot 0 cannot be a tail')

    if self.knots[idx - 1] == self.knots[idx]:
      return

    if self.knots[idx - 1].x == self.knots[idx].x + 2:  # head go ahead
      self.knots[idx].x += 1
      self.knots[idx].y = self.knots[idx - 1].y

    if self.knots[idx - 1].x == self.knots[idx].x - 2:  # head is backing away
      self.knots[idx].x -= 1
      self.knots[idx].y = self.knots[idx - 1].y

    if self.knots[idx - 1].y == self.knots[idx].y + 2:  # head is ahead
      self.knots[idx].y += 1
      self.knots[idx].x = self.knots[idx - 1].x

    if self.knots[idx - 1].y == self.knots[idx].y - 2:  # head is below
      self.knots[idx].y -= 1
      self.knots[idx].x = self.knots[idx - 1].x

    self.ts.add((self.knots[idx].x, self.knots[idx].y))

  def play(self, lines):
    for line in lines:
      print('process line', line)
      direction, steps = line.split()
      for i in range(int(steps)):
        self.move_head(direction)
        print('h moves to', self.knots[0])

        for idx in range(1, len(self.knots)):
          self.move_tail(idx)
          print('t[%d] moves to' % idx, self.knots[idx])

        self.draw()


def main():
  lines = read_input_file(sys.argv[1])
  print('Got', len(lines), 'lines')

  b = Board()
  b.play(lines)
  print('visited', len(b.ts))


if __name__ == '__main__':
  main()
