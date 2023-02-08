#!/usr/bin/env python3

import gi
import sys

from tagdb import TagDB

gi.require_version('Gtk', '3.0')
from gi.repository import Gtk  # , Gdk, GdkPixbuf  # noqa: E402

GLADE_PATH = 'image_tagger.glade'


class MainApp:
  def __init__(self):
    self.db = TagDB('images.sqlite')
    self.filename = 'fake.jpg'

    self.builder = Gtk.Builder()
    self.builder.add_from_file(GLADE_PATH)

    self.tag_store = self.builder.get_object('tag_store')
    self.tag_store.append(['hi'])
    self.tag_store.append(['there'])

    self.all_tags_store = self.builder.get_object('all_tags_store')
    self.all_tags_store.append(['some'])
    self.all_tags_store.append(['other'])
    self.all_tags_store.append(['tag'])

    tag_entry = self.builder.get_object('tag_entry')
    tag_entry.connect('activate', self.tag_entry_activated)

    # For some reason, the glade must set -1 and override here.
    completion = self.builder.get_object('tag_completion')
    completion.set_text_column(0)

    window = self.builder.get_object('main_window')
    window.connect('destroy', Gtk.main_quit)
    window.show_all()

  def run(self):
    Gtk.main()

  def tag_entry_activated(self, entry):
    text = entry.get_text()
    entry.delete_text(0, -1)

    if not text:
      return

    if text == 'clear':
      self.tag_store.clear()
    else:
      self.tag_store.append((text,))

    self.all_tags_store.append((text,))
    print('all tags:', self.all_tags())

  def all_tags(self):
    res = []
    it = self.all_tags_store.get_iter_first()
    while (it):
      res.append(self.all_tags_store.get(it, 0)[0])
      it = self.all_tags_store.iter_next(it)

    return res


def main(args):
  m = MainApp()
  m.run()


if __name__ == '__main__':
  main(sys.argv[1:])
