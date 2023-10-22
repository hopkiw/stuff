#!/usr/bin/env python3
"""Module docstring."""

import pyglet
import pyglet.gl
from pyglet.window import mouse, key


window = pyglet.window.Window(visible=False, resizable=True)


@window.event
def on_draw():
    window.clear()
    pyglet.gl.glTexParameteri(pyglet.gl.GL_TEXTURE_2D,
                              pyglet.gl.GL_TEXTURE_MAG_FILTER,
                              pyglet.gl.GL_NEAREST)
    pyglet.gl.glTexParameteri(pyglet.gl.GL_TEXTURE_2D,
                              pyglet.gl.GL_TEXTURE_MIN_FILTER,
                              pyglet.gl.GL_NEAREST)
    batch.draw()


@window.event
def on_mouse_press(x, y, button, modifiers):
    if button == mouse.LEFT:
      print(f'The left mouse button was pressed at ({x},{y}).')
    elif button == mouse.RIGHT:
      print(f'The right mouse button was pressed at ({x},{y}).')


@window.event
def on_key_press(symbol, modifiers):
    if symbol == key.ENTER:
        print('enter key pressed')
    else:
        print(symbol, 'key pressed')


if __name__ == '__main__':
    swampimg = pyglet.image.load(
        'sprites2/forest_ v1.3/swamp_.png')
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

    decorimg = pyglet.image.load(
        'sprites2/forest_ v1.3/swampDecoration_0.png')
    decor = pyglet.image.ImageGrid(decorimg, 11, 11)

    # 90 = grass decoration
    # 78 = top of tree
    # 67 = bottom of tree

    puddleimg = pyglet.image.load(
        'sprites2/forest_ v1.3/puddle_.png')
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
    pyglet.gl.glEnable(pyglet.gl.GL_BLEND)
    pyglet.gl.glBlendFunc(pyglet.gl.GL_SRC_ALPHA,
                          pyglet.gl.GL_ONE_MINUS_SRC_ALPHA)

    window.width = 400
    window.height = 400
    window.set_visible()

    batch = pyglet.graphics.Batch()

    tilesets = (swamp, decor, puddle)
    picture = [
        [(0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182)],
        [(0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182)],
        [(0, 182), (0, 182), (0, 200), (0, 167), (0, 167), (0, 167), (0, 201),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182)],
        [(0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182)],
        [(0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182)],
        [(0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182)],
        [(0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182)],
        [(0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182)],
        [(0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182)],
        [(0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182)],
        [(0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182)],
        [(0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182)],
        [(0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182)],
        [(0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182)],
        [(0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182)],
        [(0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182)],
        [(0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182)],
        [(0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182)],
        [(0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182)],
        [(0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182)],
        [(0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182)],
        [(0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182)],
        [(0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182)],
        [(0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182)],
        [(0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182), (0, 182),
         (0, 182), (0, 182), (0, 182), (0, 77)],
    ]

    tiles = []
    for x in range(25):
        for y in range(25):
            thistile = picture[y][x]
            tileset = tilesets[thistile[0]]
            tile = pyglet.sprite.Sprite(tileset[thistile[1]],
                                        x=x * 16, y=y * 16, batch=batch)
            tile.scale = 4
            tiles.append(tile)

    pyglet.app.run()