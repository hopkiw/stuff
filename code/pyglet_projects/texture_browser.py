#!/usr/bin/env python3
"""Module docstring."""

import pyglet
from pyglet.gl import glEnable, glBlendFunc, GL_BLEND, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA
from pyglet.window import mouse, key


window = pyglet.window.Window(visible=False, resizable=True)
idx = 167
bgtile = None


@window.event
def on_draw():
    # background.blit_tiled(0, 0, 0, window.width, window.height)
    # img.blit(window.width // 2, window.height // 2, 0)
    window.clear()
    tile.draw()


@window.event
def on_mouse_press(x, y, button, modifiers):
    if button == mouse.LEFT:
      print(f'The left mouse button was pressed at ({x},{y}).')
    elif button == mouse.RIGHT:
      print(f'The right mouse button was pressed at ({x},{y}).')


@window.event
def on_key_press(symbol, modifiers):
    global idx
    if symbol == key.RIGHT:
        idx += 1
    elif symbol == key.LEFT:
        idx -= 1
    else:
        return
    print('idx is', idx)
    global tile
    tile = pyglet.sprite.Sprite(swamp[idx], x=50, y=50)
    tile.scale = 4
    on_draw()


if __name__ == '__main__':
    swampimg = pyglet.image.load('sprites2/forest_ v1.3/swamp_.png')
    swamp = pyglet.image.ImageGrid(swampimg, 36, 15)

    # 200 = upper left
    # 201 = upper right
    # 167 = top edge of island
    # 185 = lower left corner of island
    # 186 = lower right corner of island
    # 197 = bottom edge
    # 136 = right edge
    # 137 = left edge
    # 182 = pure water
    # 77 = pure grass

    decorimg = pyglet.image.load('sprites2/forest_ v1.3/swampDecoration_0.png')
    decor = pyglet.image.ImageGrid(decorimg, 11, 11)

    # 90 = grass decoration
    # 78 = top of tree
    # 67 = bottom of tree

    puddleimg = pyglet.image.load('sprites2/forest_ v1.3/puddle_.png')
    puddle = pyglet.image.ImageGrid(puddleimg, 6, 15)

    # 31 = lower left
    # 32 = bottom
    # 33 = lower right
    # 46 = left
    # 47 = water
    # 48 = right
    # 61 = upper left
    # 62 = top
    # 63 = upper right

    # Enable alpha blending, required for image.blit.
    glEnable(GL_BLEND)
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)

    window.width = 400
    window.height = 400
    window.set_visible()

    tile = pyglet.sprite.Sprite(swamp[idx], x=50, y=50)
    tile.scale = 4

    pyglet.app.run()
