#!/usr/bin/env python3
"""Module docstring."""

import pyglet
from pyglet.window import mouse


window = pyglet.window.Window()

image = pyglet.image.load('sprites/3/Hurt.png')
player_seq = pyglet.image.ImageGrid(image, 1, 2)
regular = pyglet.sprite.Sprite(player_seq[0], x=200, y=200)
regular.scale = 4
hurt = pyglet.sprite.Sprite(player_seq[1], x=200, y=200)
hurt.scale = 4

counter = 0


@window.event
def on_draw():
    global counter
    window.clear()
    counter = (counter + 1) % 400
    if counter in (20, 21, 22):
        hurt.draw()
    else:
        regular.draw()


@window.event
def on_mouse_press(x, y, button, modifiers):
    if button == mouse.LEFT:
      print(f'The left mouse button was pressed at ({x},{y}).')
    elif button == mouse.RIGHT:
      print(f'The right mouse button was pressed at ({x},{y}).')


pyglet.app.run()
