from pyglet.gl import glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_TEXTURE_MIN_FILTER, GL_NEAREST
import pyglet


window = pyglet.window.Window()


@window.event
def on_draw():
    window.clear()  # clears the screen
    # The following two lines will change how textures are scaled.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST)
    batch.draw()


if __name__ == '__main__':
    batch = pyglet.graphics.Batch()

    image = pyglet.image.load('sprites/3/Hurt.png')
    player_seq = pyglet.image.ImageGrid(image, 1, 2)
    sprite = pyglet.sprite.Sprite(player_seq[0], batch=batch)
    sprite.scale = 4

    pyglet.app.run()
