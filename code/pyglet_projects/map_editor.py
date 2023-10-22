#!/usr/bin/env python3
"""Module docstring."""

import pyglet

from pyglet.window import key

WIDTH = 11
HEIGHT = 8


window = pyglet.window.Window(visible=False, resizable=True)


@window.event
def on_draw():
    if keys[key.UP]:
        maplabel.x += 1

    if mode == 0:
        mapoutline.visible = True
        maplabel.visible = True
        maptile.visible = True

        choiceoutline.visible = False
        choicelabel.visible = False
        choicetile.visible = False
    else:
        mapoutline.visible = False
        maplabel.visible = False
        maptile.visible = False

        choiceoutline.visible = True
        choicelabel.visible = True
        choicetile.visible = True

    window.clear()
    ceiling = (HEIGHT * 64) + 200
    choicelabel.text = f'map:{curmap} idx:{idx}'
    maplabel.text = f'x:{maptile.x} y:{maptile.y} ceil:{ceiling}'
    batch.draw()


@window.event
def on_mouse_press(x, y, button, modifiers):
    if button == pyglet.window.mouse.LEFT:
      print(f'The left mouse button was pressed at ({x},{y}).')
    elif button == pyglet.window.mouse.RIGHT:
      print(f'The right mouse button was pressed at ({x},{y}).')


@window.event
def on_key_press(symbol, modifiers):
    global idx, mode, imagegrids, curmapname, maptile, choicetile

    if symbol == pyglet.window.key.TAB:
        mode = (mode + 1) % 2
        return

    if mode == 0:
        if symbol == pyglet.window.key.RIGHT:
            x = maptile.x + 64
            if x > ((WIDTH - 1) * 64) + 70:
                x = 70
            maptile.x = x
        elif symbol == pyglet.window.key.LEFT:
            x = maptile.x - 64
            if x < 70:
                x = ((WIDTH - 1) * 64) + 70
            maptile.x = x
        elif symbol == pyglet.window.key.UP:
            ceiling = ((HEIGHT - 1) * 64) + 220
            y = maptile.y + 64
            if y > ceiling:
                y = 220
            maptile.y = y
        elif symbol == pyglet.window.key.DOWN:
            y = maptile.y - 64
            if y < 220:
                y = ((HEIGHT - 1) * 64) + 220
            maptile.y = y
        else:
            return
    else:
        print('curmapname', curmapname)
        maps = list(imagegrids.keys())
        curmapidx = maps.index(curmapname)

        if symbol == pyglet.window.key.RIGHT:
            idx += 1
            # TODO: max? len(map)
        elif symbol == pyglet.window.key.LEFT:
            idx -= 1
            if idx < 0:
                idx = 0
        elif symbol == pyglet.window.key.UP:
            print('curmapidx', curmapidx)
            curmapidx = (curmapidx + 1) % len(maps)
            print('curmapidx', curmapidx)
            curmapname = maps[curmapidx]
            print('curmapname', curmapname)
            # choicetile.image = curmap[idx]
        elif symbol == pyglet.window.key.DOWN:
            curmapidx = curmapidx - 1
            if curmapidx < 0:
                curmapidx = len(maps) - 1
            curmap = imagegrids[maps[curmapidx]]
            choicetile.image = curmap[idx]
        else:
            return
        curmap = imagegrids[curmapname]
        choicetile = pyglet.sprite.Sprite(curmap[idx], x=choicetile.x,
                                          y=choicetile.y)


if __name__ == '__main__':
    imagegrids = {}

    swampimg = pyglet.image.load(
        'sprites2/forest_ v1.3/swamp_.png')
    imagegrids['swamp'] = pyglet.image.ImageGrid(swampimg, 36, 15)

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
    imagegrids['decor'] = pyglet.image.ImageGrid(decorimg, 11, 11)

    # 90 = grass decoration
    # 78 = top of tree
    # 67 = bottom of tree

    puddleimg = pyglet.image.load(
        'sprites2/forest_ v1.3/puddle_.png')
    imagegrids['puddle'] = pyglet.image.ImageGrid(puddleimg, 6, 15)

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

    # Change scaling
#    pyglet.gl.glTexParameteri(pyglet.gl.GL_TEXTURE_2D,
#                              pyglet.gl.GL_TEXTURE_MAG_FILTER,
#                              pyglet.gl.GL_NEAREST)
#    pyglet.gl.glTexParameteri(pyglet.gl.GL_TEXTURE_2D,
#                              pyglet.gl.GL_TEXTURE_MIN_FILTER,
#                              pyglet.gl.GL_NEAREST)

    pyglet.image.Texture.default_min_filter = pyglet.gl.GL_NEAREST
    pyglet.image.Texture.default_mag_filter = pyglet.gl.GL_NEAREST

    batch = pyglet.graphics.Batch()

    curmapname = list(imagegrids.keys())[1]
    curmap = imagegrids[curmapname]
    idx = 0

    mapoutline = pyglet.shapes.Rectangle(50, 200, 750, 550, color=(255, 0, 0),
                                         batch=batch)
    mapoutline2 = pyglet.shapes.Rectangle(70, 220, 710, 510, color=(0, 0, 0),
                                          batch=batch)

    choiceoutline = pyglet.shapes.Rectangle(850, 430, 320, 320,
                                            color=(0, 255, 0), batch=batch)
    choiceoutline2 = pyglet.shapes.Rectangle(870, 450, 280, 280,
                                             color=(0, 0, 0), batch=batch)

    maplabel = pyglet.text.Label('', font_name='Cooper', font_size=16,
                                 x=80, y=150, batch=batch)
    choicelabel = pyglet.text.Label('', font_name='Cooper', font_size=16,
                                    x=890, y=390, batch=batch)

    maptile = pyglet.shapes.Rectangle(70, 220, 64, 64, color=(255, 255, 255),
                                      batch=batch)
    choicetile = pyglet.sprite.Sprite(imagegrids[curmapname][20], x=990, y=550, batch=batch)
    choicetile.scale = 4
    # choicetile = pyglet.sprite.Sprite(choiceimg, x=990, y=550, batch=batch)

    mode = 0
    x = 0
    y = 0

    keys = pyglet.window.key.KeyStateHandler()
    window.push_handlers(keys)

    window.width = 1400
    window.height = 800
    window.set_visible()

    pyglet.app.run()
