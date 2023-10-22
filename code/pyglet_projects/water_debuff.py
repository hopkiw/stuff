#!/usr/bin/env python3
"""Module docstring."""

import pyglet
from pyglet.window import mouse, key

WETTING_RATE = 1 / 2.0
DRYING_RATE = 10 / 3.0
COOLING_RATE = 1 / 6.0
WARMING_RATE = 1  # 10 warmth / 10 seconds
HURTING_RATE = 5 / 1.0
WETNESS_THRESHOLD = 65

window = pyglet.window.Window()


@window.event
def on_draw():
    window.clear()
    bgsprite.draw()
    sprite.draw()
    label.draw()


@window.event
def on_mouse_press(x, y, button, modifiers):
    if button == mouse.LEFT:
      print(f'The left mouse button was pressed at ({x},{y}).')
    elif button == mouse.RIGHT:
      print(f'The right mouse button was pressed at ({x},{y}).')


@window.event
def on_key_press(symbol, modifiers):
    global in_water

    if symbol == key.ENTER:
        print('enter key pressed')
    elif symbol == key.RIGHT:
        sprite.x = 300
        in_water = True
    elif symbol == key.LEFT:
        sprite.x = 50
        in_water = False
    else:
        print(symbol, 'key pressed')


def status_update(dt):
    global wetness
    global in_water
    global body_temp
    global health
    global hypothermia

    if in_water:
        wetness = min(wetness + WETTING_RATE, 100)
    else:
        wetness = max(wetness - DRYING_RATE, 0)

    if wetness:
        if wetness > WETNESS_THRESHOLD:
            body_temp = max(body_temp - (2 * COOLING_RATE), 0)
        else:
            body_temp = max(body_temp - COOLING_RATE, 0)
    else:
        body_temp = min(body_temp + WARMING_RATE, 100)

    if body_temp == 0:
        hypothermia = True

    if hypothermia:
        if body_temp >= 40:
            hypothermia = False
            sprite.color = (255, 255, 255)
        else:
            sprite.color = (100, 100, 255)
            health = max(health - HURTING_RATE, 0)

    label.text = (f'Wetness: {int(wetness)} Body temp: {int(body_temp)} '
                  f'Health: {int(health)}')


if __name__ == '__main__':
    in_water = False
    hypothermia = False
    body_temp = 100
    health = 100
    wetness = 0

    bg = pyglet.image.load('new.png')
    bgsprite = pyglet.sprite.Sprite(bg)
    bgsprite.scale = 2

    image = pyglet.image.load('sprites/3/Hurt.png')
    player_seq = pyglet.image.ImageGrid(image, 1, 2)
    sprite = pyglet.sprite.Sprite(player_seq[0], x=50, y=175)
    sprite.scale = 4
    # sprite.color = (100, 100, 255)

    label = pyglet.text.Label(f'Wetness: {int(wetness)} Body temp: '
                              f'{int(body_temp)} Health: {int(health)}',
                              font_name='Cooper',
                              font_size=16,
                              x=250,
                              y=50,
                              anchor_x='center',
                              anchor_y='center')

    pyglet.clock.schedule_interval(status_update, 1)
    pyglet.app.run()
