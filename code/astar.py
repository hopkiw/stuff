#!/usr/bin/env python3
"""Module docstring."""

import math
import unittest

import pyglet
from pyglet import shapes
from pyglet.window import mouse


# TODO: should be able to pan and zoom on the game board.

# TODO: separate game logic from graphics.

class Tile:
  """A game tile."""

  def __init__(self, idx, x, y, size, color, batch, fgbatch):
    self.idx = idx
    self.x = x
    self.y = y
    self.size = size
    self.color = color
    self.batch = batch
    self.fgbatch = fgbatch

    self.rectangle = shapes.Rectangle(x, y, size, size, color=color,
                                      batch=batch)
    self.make_labels()

    self.black = False

    self.d = 0
    self.s = 0
    self.parent = 0

  def make_labels(self):
    self.label_1 = pyglet.text.Label(' ', font_name='Times New Roman',
                                     font_size=20, x=self.x+10, y=self.y+70,
                                     batch=self.fgbatch)
    self.label_2 = pyglet.text.Label(' ', font_name='Times New Roman',
                                     font_size=20, x=self.x+70, y=self.y+70,
                                     batch=self.fgbatch)
    self.label_3 = pyglet.text.Label(f' ', font_name='Times New Roman',
                                     font_size=24, x=self.x+35, y=self.y+25,
                                     batch=self.fgbatch)

  def draw(self):
    if self.black:
      self.rectangle.color = (20, 20, 20)
      return

    self.rectangle.color = self.color
    self.label_1.text = f'{self.d}'
    self.label_2.text = f'{self.s}'
    self.label_3.text = f'{self.s+self.d}'


class Game:
  """A game window for displaying the A* pathing algorithm."""

  def __init__(self, rows=3, columns=3):
    self.rows = rows
    self.columns = columns

    # size values for tiles
    self.size = 50
    self.padding = 10

    # self.dragging = False
    # Storage for game tiles
    self.tiles = []
    # self.circles = []  # storage for debug circle shapes

    # pyglet objects for drawing the game.
    # window
    self.window = pyglet.window.Window(1320, 770)
    # batch for shapes
    self.batch = pyglet.graphics.Batch()
    # batch for foreground text
    self.fgbatch = pyglet.graphics.Batch()

    # A-star implementation objects
    self.open_list = []
    self.closed_list = []
    self.current = None
    self.src = None
    self.dest = None

    # Create all tiles
    self.make_tiles()

    # Event handlers
    self.window.push_handlers(on_draw=self.on_draw)
    self.window.push_handlers(on_mouse_press=self.on_mouse_press)
    # self.window.push_handlers(on_mouse_drag=self.on_mouse_drag)
    # self.window.push_handlers(on_mouse_release=self.on_mouse_release)

  def make_tiles(self):
    """Populate self.tiles."""
    x = 0
    y = 0
    for idx in range(self.rows * self.columns):
      self.tiles.append(Tile(idx, x, y, self.size, (255, 255, 255), self.batch,
                             self.fgbatch))
      if (idx+1) % self.columns == 0:
        x = 0
        y = y + self.size + self.padding
      else:
        x = x + self.size + self.padding

  def draw_circle(self, x, y):
    circle = shapes.Circle(x, y, radius=10, color=(50, 225, 30),
                           batch=self.fgbatch)
    self.circles.append(circle)
    pyglet.clock.schedule_once(self.delete_circle, 0.65, circle)

  def delete_circle(self, dt, circle):
    """Delete an on-click circle."""
    circle.delete()
    self.circles.remove(circle)

  def on_draw(self):
    self.window.clear()
    self.batch.draw()
    self.fgbatch.draw()

  def on_mouse_press(self, x, y, button, modifiers):
    """On mouse press."""
    if button == mouse.LEFT:
      print(f'The left mouse button was pressed at ({x},{y}).')
      # self.draw_circle(x, y)

      if x < (self.size+self.padding) * self.columns and y < (self.size+self.padding) * self.rows:
        idx = self.tile(x, y)
        print(f'got tile {idx}')
        tile = self.tiles[idx]

        if not self.src:
          print(f'set src to {idx}')
          self.src = idx
          tile.rectangle.color = (55, 55, 255)
          tile.label_3.text = 'S'

          return

        if not self.dest:
          print(f'set dest to {idx}')
          self.dest = idx
          tile.rectangle.color = (55, 55, 255)
          tile.label_3.text = 'D'

          return

        if tile.black:
          tile.black = False
          tile.rectangle.color = (255, 255, 255)
        else:
          tile.black = True
          tile.rectangle.color = (20, 20, 20)

    elif button == mouse.RIGHT:
      if not self.current or self.current != self.dest:
        self.start_pathing()
        print(f'current tile {self.current}')
        for idx in self.closed_list:
          self.tiles[idx].rectangle.color = (255,55,55)
        for idx in self.open_list:
          if idx == self.dest:
            continue
          tile = self.tiles[idx]
          tile.rectangle.color = (55,55,255)
          tile.label_1.text = f'{tile.d}'
          tile.label_2.text = f'{tile.s}'
          tile.label_3.text = f'{tile.d+tile.s}'
      self.tiles[self.current].rectangle.color = (55,255,55)

  def tile(self, x, y):
    """Get tile idx from x,y position."""
    row = int(y / (self.size+self.padding))
    col = int(x / (self.size+self.padding))

    return (row*self.columns) + col

  def rowcol(self, idx):
    """Get row and column from idx."""
    return int(idx / self.columns), idx % self.columns

  def get_neighbors(self, current):
    neighbors = (
        (14, current - self.columns - 1),
        (10, current - self.columns),
        (14, current - self.columns + 1),
        (10, current - 1),
        (10, current + 1),
        (14, current + self.columns - 1),
        (10, current + self.columns),
        (14, current + self.columns + 1)
    )

    current_cost = self.tiles[self.current].s
    res = []

    for cost, idx in neighbors:
      if idx < 0:
        continue
      if idx > len(self.tiles):
        continue
      if (current+1) % self.columns == 0 and idx % self.columns == 0:
        continue
      if current % self.columns == 0 and (idx+1) % self.columns == 0:
        continue

      res.append((cost+current_cost, idx))

    return res

  def get_lowest_t(self):
    # Pick the tile with the lowest T score (S+D scores). If there's a tie,
    # pick the one with the lowest D score.
    if len(self.open_list) == 1:
      return self.open_list[0]

    sortby = {}
    for idx in self.open_list:
      tile = self.tiles[idx]
      t = tile.s+tile.d
      if t in sortby:
        sortby[t].append(idx)
      else:
        sortby[t] = [idx]
    tiles_with_lowest_t = sorted(sortby.items())[0][1]

    if len(tiles_with_lowest_t) == 1:
      return tiles_with_lowest_t[0]

    # If there were multiple with the same T value, sort again by S value.
    sortby = {}
    for idx in tiles_with_lowest_t:
      tile = self.tiles[idx]
      if tile.d in sortby:
        sortby[tile.d].append(idx)
      else:
        sortby[tile.d] = [idx]

    # Return the first entry with lowest S value.
    return sorted(sortby.items())[0][1][0]

  def distance(self, src, dest):
    dest_row, dest_col = self.rowcol(dest)
    src_row, src_col = self.rowcol(src)
    dx = abs(dest_col - src_col)
    dy = abs(dest_row - src_row)

    return int(math.sqrt(dx**2 + dy**2) * 10)

  def start_pathing(self):
    """Start pathing."""
    if self.current:
      if self.current == self.dest:
        print(f'called start_pathing but pathing is done')
        idx = self.current
        while idx != self.src:
          print(f'setting tile {idx} green')
          self.tiles[idx].rectangle.color = (55, 255, 55)
          idx = self.tiles[idx].parent

        return True
      orig = self.current
      self.open_list.remove(self.current)
      self.closed_list.append(self.current)
      self.current = self.get_lowest_t()
      self.tiles[self.current].parent = orig
    else:
      self.open_list = [self.src]
      self.current = self.get_lowest_t()


    # Evaluate neighbors
    for cost, idx in self.get_neighbors(self.current):
      tile = self.tiles[idx]
      if tile.black or idx in self.closed_list:
        continue

      if idx not in self.open_list or cost < tile.s:
        tile.s = cost  # update-or-set
        tile.d = self.distance(idx, self.dest)  # set or overwrite
        tile.parent = self.current  # for tracking path. is this right? idx is
        # usually only known to the game. now a list is a linked list?
        if idx not in self.open_list:
          self.open_list.append(idx)

    return False



def main():
  g = Game(7, 12)
  pyglet.app.run()


def test_distance():
  g = Game(7, 12)
  assert g.distance(30, 52) == 28


def test_rowcol():
  g = Game(7, 12)
  assert g.rowcol(83) == (6,11)


def test_get_neighbors():
  g = Game(7, 12)
  res = g.get_neighbors(30)
  assert res == [(14, 17), (10, 18), (14, 19), (10, 29), (10, 31), (14, 41),
                 (10, 42), (14, 43)]
  for cost, idx in res:
    g.tiles[idx].s = cost
  res = g.get_neighbors(29)
  assert res == [(14, 16), (24, 17), (24, 18), (10, 28), (10, 30), (14, 40),
                 (24, 41), (24, 42)]


def test_get_lowest_t():
  g = Game(7, 12)
  g.src = 30
  dest = 52  # let's take the first step toward 52
  g.open_list = [17,18,19,29,31,41,42,43]
  print(f'set g.open_list to {g.open_list}')
  for f in g.open_list:
    dist = g.distance(f, dest)
    print(f'idx {f} dist to dest {dist}')
    g.tiles[f].d = dist
  assert g.get_lowest_t() == 41  # this is the correct next step
  # this doesn't have any side effects on state
  g.src = 16
  dest = 42
  g.open_list = [3,4,5,15,16,17,27,28,29]
  print(f'set g.open_list to {g.open_list}')
  for f in g.open_list:
    dist = g.distance(f, dest)
    print(f'idx {f} dist to dest {dist}')
    g.tiles[f].d = dist
  assert g.get_lowest_t() == 29


def test_start_pathing():
  tc = unittest.TestCase()
  g = Game(7, 12)
  g.src = 30
  g.dest = 52
  tc.assertFalse( g.start_pathing() )
  tc.assertEqual( g.current, 30 )
  tc.assertEqual( [30], g.closed_list )
  tc.assertEqual( [17, 18, 19, 29, 31, 41, 42, 43], g.open_list )
  tc.assertFalse( g.start_pathing() )
  tc.assertEqual( g.current, 41 )
  tc.assertEqual( [30, 41], g.closed_list )
  tc.assertEqual( [17, 18, 19, 29, 31, 42, 43, 28, 40, 52, 53, 54], g.open_list )
  tc.assertTrue( g.start_pathing() )
  tc.assertEqual( g.current, g.dest )
  tc.assertEqual( [30, 41], g.closed_list )
  tc.assertEqual( [17, 18, 19, 29, 31, 42, 43, 28, 40, 52, 53, 54], g.open_list )


if __name__ == '__main__':
  main()
  #test_distance()
  #test_rowcol()
  #test_get_neighbors()
  #test_get_lowest_t()
  #test_start_pathing()
  #print('passed')
