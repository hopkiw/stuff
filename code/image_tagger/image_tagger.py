#!/usr/bin/env python3

import gi
import sys

from tagdb import TagDB

gi.require_version('Gtk', '3.0')
from gi.repository import Gtk, Gdk, GdkPixbuf  # noqa: E402

GLADE_PATH = 'image_tagger.glade'


class MainApp:
  def __init__(self, args):
    self.db = TagDB('images.sqlite')
    self.filename = 'fake.jpg'

    self.builder = Gtk.Builder()
    self.builder.add_from_file(GLADE_PATH)

    self.tag_store = self.builder.get_object('tag_store')

    self.all_tags_store = self.builder.get_object('all_tags_store')
    for tag in self.db.all_tags():
      self.all_tags_store.append([tag])

    tag_entry = self.builder.get_object('tag_entry')
    tag_entry.connect('activate', self.tag_entry_activated)

    # For some reason, the glade must set -1 and override here.
    completion = self.builder.get_object('tag_completion')
    completion.set_text_column(0)

    self.window = self.builder.get_object('main_window')
    self.window.connect('key-press-event', self.key_press_event)
    self.window.connect('destroy', Gtk.main_quit)

    self.args = args
    self.index = 0

    self.image = self.builder.get_object('image')

    if self.args:
      self.update()

    self.window.show_all()

  def run(self):
    Gtk.main()

  def tag_entry_activated(self, entry):
    text = entry.get_text()
    entry.delete_text(0, -1)

    if not text:
      return

    self.tag_store.append((text,))
    self.all_tags_store.append((text,))

  def all_tags(self):
    res = []
    it = self.all_tags_store.get_iter_first()
    while (it):
      res.append(self.all_tags_store.get(it, 0)[0])
      it = self.all_tags_store.iter_next(it)

    return res

  def update(self):
    self.tag_store.clear()
    tags = self.db.get_tags(self.args[self.index])
    for tag in tags:
      self.tag_store.append([tag])
    # from IPython import embed;embed()

    rect = self.window.get_allocation()
    if rect.x < 0 or rect.y < 0 or rect.width < 1 or rect.height < 1:
      return

    self.zoom_to_size(rect.width - 2, rect.height - 2)

  def zoom_to_size(self, width, height):
    pixbuf = GdkPixbuf.Pixbuf.new_from_file(self.args[self.index])
    pixbuf_width = pixbuf.get_width()
    pixbuf_height = pixbuf.get_height()

    if width / float(height) >= pixbuf_width / float(pixbuf_height):
      new_width = pixbuf_width * height // pixbuf_height
      new_height = height
    else:
      new_width = width
      new_height = pixbuf_height * width // pixbuf_width

    pixbuf = pixbuf.scale_simple(new_width, new_height,
                                 GdkPixbuf.InterpType.BILINEAR)
    self.image.clear()
    self.image.set_from_pixbuf(pixbuf)

  def key_press_event(self, widget, event):
    keyval = event.keyval
    keyval_name = Gdk.keyval_name(keyval)

    # state = event.state
    if keyval_name == 'Right':
      if self.index == len(self.args) - 1:
        return
      self.index = min(self.index + 1, len(self.args) - 1)
    elif keyval_name == 'Left':
      if self.index == 0:
        return
      self.index = max(self.index - 1, 0)
    else:
      return

    self.update()


if __name__ == '__main__':
  m = MainApp(sys.argv[1:])
  m.run()
