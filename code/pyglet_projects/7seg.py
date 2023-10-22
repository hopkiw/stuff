#!/usr/bin/env python3
"""Module docstring."""

import pyglet
from pyglet import shapes
from pyglet.window import mouse

# TODO: grey background, nearly-transparent 'off' states
# TODO: multiple digits and applications for them
# TODO: use ordered group instead of multiple batches

# Display a 7-segment LED/LCD and a colored input-to-output chart
#
#                     a b c d e f g
#                    -------------
#      a          0 | x x x x x x _
#     ---         1 | _ x x _ _ _ _
#  f |   | b      2 | x x _ x x _ x
#    | g |        3 | x x x x _ _ x
#     ---         4 | _ x x _ _ x x
#  e |   | c      5 | x _ x x _ x x
#    |   |        6 | x _ x x x x x
#     ---         7 | x x x _ _ _ _
#      d          8 | x x x x x x x
#                 9 | x x x x _ x x

WHITE = (128, 128, 128)
RED = (255, 0, 0)

SEGMENTS = [
    'abcdefg',  # 0
    'bc',       # 1
    'abdeg',    # 2
    'abcdg',    # 3
    'bcfg',     # 4
    'acdfg',    # 5
    'acdefg',   # 6
    'abc',      # 7
    'abcdefg',  # 8
    'abcdfg',   # 9
]


class Segment:
  """A display segment."""

  def __init__(self, x, y, width, height, vertical, color, batch, fgbatch):
    hatcolor = shoecolor = color
    if vertical:
      x = x - (width / 2.0)
      y = y + (width / 2.0)
      self.rectangle = shapes.Rectangle(x, y, width, height, color=color,
                                        batch=batch)
      trix = x
      triy = y + height
      self.hat = shapes.Triangle(trix, triy,
                                 trix + (width / 2.0), triy + (width / 2.0),
                                 trix + width, triy,
                                 color=hatcolor, batch=batch)
      trix = x
      triy = y
      self.shoe = shapes.Triangle(trix, triy,
                                  trix + (width / 2.0), triy - (width / 2.0),
                                  trix + width, triy,
                                  color=shoecolor, batch=batch)
    else:
      x = x + (width / 2.0)
      y = y - (width / 2.0)
      self.rectangle = shapes.Rectangle(x, y, height, width, color=color,
                                        batch=batch)
      trix = x
      triy = y
      self.hat = shapes.Triangle(trix, triy,
                                 trix - (width / 2.0), triy + (width / 2.0),
                                 trix, triy + width,
                                 color=hatcolor, batch=batch)
      trix = x + height
      triy = y
      self.shoe = shapes.Triangle(trix, triy,
                                  trix + (width / 2.0), triy + (width / 2.0),
                                  trix, triy + width,
                                  color=shoecolor, batch=batch)

  def off(self):
    self.rectangle.color = WHITE
    self.hat.color = WHITE
    self.shoe.color = WHITE

  def on(self):
    self.rectangle.color = RED
    self.hat.color = RED
    self.shoe.color = RED


class Display:
  """A game window for displaying the A* pathing algorithm."""

  def __init__(self):
    self.segments = {}
    self.number = 0

    # pyglet objects for drawing the game.
    # window
    self.window = pyglet.window.Window(770, 770)
    # batch for shapes
    self.batch = pyglet.graphics.Batch()
    # batch for foreground text
    self.fgbatch = pyglet.graphics.Batch()

    self.make_segments()
    pyglet.clock.schedule_interval(self.update, 0.8)

    # Event handlers
    self.window.push_handlers(on_draw=self.on_draw)
    self.window.push_handlers(on_mouse_press=self.on_mouse_press)

  def make_segments(self):
    topx = 200
    topy = 610
    width = 40
    height = 110
    ordering = ['a', 'g', 'd', 'f', 'e', 'b', 'c']

    idx = 0
    # Segments a, g, d
    for i in range(3):
      segment = Segment(topx, topy - (i * (width + height)),
                        width, height, False, WHITE,
                        self.batch, self.fgbatch)
      self.segments[ordering[idx]] = segment
      idx = idx + 1

    # Segments f, e
    for i in range(1, 3):
      segment = Segment(topx, topy - (i * (width + height)),
                        width, height, True, WHITE,
                        self.batch, self.fgbatch)
      self.segments[ordering[idx]] = segment
      idx = idx + 1

    # Segments b, c
    for i in range(1, 3):
      segment = Segment(topx + width + height, topy - (i * (width + height)),
                        width, height, True, WHITE,
                        self.batch, self.fgbatch)
      self.segments[ordering[idx]] = segment
      idx = idx + 1

  def update(self, dt):
    print('update was called with val', dt)
    for segment in self.segments.values():
      segment.off()
    self.number = (self.number + 1) % 10
    for segment in self.getsegments(self.number):
      self.segments[segment].on()

  def getsegments(self, num):
    return SEGMENTS[num]

  def on_draw(self):
    self.window.clear()
    self.batch.draw()
    self.fgbatch.draw()

  def on_mouse_press(self, x, y, button, modifiers):
    """On mouse press."""
    if button == mouse.LEFT:
      print(f'The left mouse button was pressed at ({x},{y}).')
    elif button == mouse.RIGHT:
      print(f'The right mouse button was pressed at ({x},{y}).')


def main():
  d = Display()
  input()
  pyglet.app.run()


if __name__ == '__main__':
  main()
