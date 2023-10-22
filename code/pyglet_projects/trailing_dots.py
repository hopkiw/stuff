#!/usr/bin/env python3
"""Module docstring."""

import pyglet


# TODO: should be able to pan and zoom on the game board.

# TODO: separate game logic from graphics.

COLORS = (
    (255, 255, 255),
    (255, 0, 0),
    (0, 255, 0),
    (0, 0, 255),
)


class Game:
  """A game window for displaying the A* pathing algorithm."""

  def __init__(self):
    self.window = pyglet.window.Window(1320, 770)
    self.batch = pyglet.graphics.Batch()

    self.circles = tuple(
        pyglet.shapes.Circle(0, 0, 50, color=COLORS[i], batch=self.batch)
        for i in range(4)
    )
    print(f'created circles: {self.circles}')

    # Event handlers
    self.window.push_handlers(on_draw=self.on_draw)
    self.window.push_handlers(on_mouse_motion=self.on_mouse_motion)

  def on_draw(self):
    self.update_circles()
    self.window.clear()
    self.batch.draw()

  def update_circles(self):
    # the first ball follows the mouse
    #
    # each ball after the first should follow the next
    #
    # they should have a magnetic attraction and inertia, such that the first
    # ball moving quickly has a slight delay as the next ball follows.
    #
    # if the lead ball stops, the follower should return to overlap it.
    #
    # the trailing effect means the circle should follow the same path, not
    # merely head straight for the following ball.

    prevcircle = self.circles[0]
    circle = self.circles[1]

    diffx = circle.x - prevcircle.x
    diffy = circle.y - prevcircle.y

    if diffx > 40 or diffy > 40:
      velocity = 20
    elif diffx > 10 or diffy > 10:
      velocity = 10
    else:
      velocity = 2

    if diffx > 1:
      circle.x = circle.x - velocity
    elif diffx < -1:
      circle.x = circle.x + velocity

    if diffy > 1:
      circle.y = circle.y - velocity
    elif diffy < -1:
      circle.y = circle.y + velocity

  def on_mouse_motion(self, x, y, dx, dy):
    print(f'mouse move {x} {y} {dx} {dy}')
    firstcircle = self.circles[0]
    firstcircle.x = x
    firstcircle.y = y


def main():
  _ = Game()
  pyglet.app.run()


if __name__ == '__main__':
  main()
