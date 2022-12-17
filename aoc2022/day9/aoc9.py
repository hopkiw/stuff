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
  def __init__(self, knots=2):
    self.knots = []
    for i in range(knots):
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
          char = ' '

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
    h = self.knots[0]

    if d == 'R':
      h.x += 1

      if h.x > self.max_x:
          self.max_x = h.x

    elif d == 'L':
      h.x -= 1

      if h.x < self.min_x:
        self.min_x = h.x

    elif d == 'U':
      h.y += 1

      if h.y > self.max_y:
          self.max_y = h.y

    elif d == 'D':
      h.y -= 1

      if h.y < self.min_y:
        self.min_y = h.y

  def move_tail(self, idx):
    if idx == 0:
      raise Exception('knot 0 cannot be a tail')

    if self.knots[idx - 1] == self.knots[idx]:
      return

    h = self.knots[idx - 1]
    t = self.knots[idx]

    changedx = False
    changedy = False

    if h.x == t.x + 2:  # head go ahead
      t.x += 1
      changedx = True

    elif h.x == t.x - 2:  # head is backing away
      t.x -= 1
      changedx = True

    elif h.y == t.y + 2:  # head is above
      t.y += 1
      changedy = True

    elif h.y == t.y - 2:  # head is below
      t.y -= 1
      changedy = True

    if changedx:
      if h.y > t.y:
        t.y += 1

      if h.y < t.y:
        t.y -= 1

    elif changedy:
      if h.x > t.x:
        t.x += 1

      if h.x < t.x:
        t.x -= 1

    if idx + 1 == len(self.knots):
      self.ts.add((t.x, t.y))

  def check_tail(self, idx):
    if idx == 0:
      raise Exception('knot 0 cannot be a tail')

    h = self.knots[idx - 1]
    t = self.knots[idx]

    if h == t:
      return

    if h.x == t.x:
      if abs(t.y - h.y) == 1:
        return

    if h.y == t.y:
      if abs(t.x - h.x) == 1:
        return

    if abs(t.x - h.x) == 1 and abs(t.y - h.y) == 1:
      return

    raise Exception((f't[{idx}] is too far from t[{idx - 1}]'))

  def play(self, lines, draw=True):
    for line in lines:
      if draw:
        print('process line', line)

      direction, steps = line.split()
      for i in range(int(steps)):
        self.move_head(direction)
        if draw:
          print('h moves to', self.knots[0])

        for idx in range(1, len(self.knots)):
          self.move_tail(idx)
          if draw:
            print('t[%d] moves to' % idx, self.knots[idx])

        if draw:
          self.draw()


def test_b():
  lines = [
      'R 5',
      'U 8',
      'L 8',
      'D 3',
      'R 17',
      'D 10',
      'L 25',
      'U 20'
  ]
  b = Board(10)
  b.play(lines, False)
  if len(b.ts) != 36:
    raise Exception('Got', len(b.ts), 'wanted 36')


def main():
  lines = read_input_file(sys.argv[1])
  print('Got', len(lines), 'lines')

  test_b()

  b = Board(10)
  b.play(lines, False)
  print(len(b.ts))


if __name__ == '__main__':
  main()
