#!/usr/bin/env python3

import urwid
from emby_tool import get_views, get_items


class EmbyBrowser:
    palette = [
        ("body", "black", "light gray"),
        ("flagged", "black", "dark green", ("bold", "underline")),
        ("focus", "light gray", "dark blue", "standout"),
        ("flagged focus", "yellow", "dark cyan", ("bold", "standout", "underline")),
        ("head", "yellow", "black", "standout"),
        ("foot", "light gray", "black"),
        ("key", "light cyan", "black", "underline"),
        ("title", "white", "black", "bold"),
        ("dirmark", "black", "dark cyan", "bold"),
        ("flag", "dark gray", "light gray"),
        ("error", "dark red", "light gray"),
    ]

    def __init__(self):
        self.view = None
        self.show_views()

    def show_views(self):
        views = get_views()
        body = []
        for view in views['Items']:
            button = urwid.Button(view['Name'])
            urwid.connect_signal(button, 'click', self.show_items, view['Id'])
            body.append(urwid.AttrMap(button, None, focus_map='reversed'))
        self.listbox = urwid.ListBox(urwid.SimpleFocusListWalker(body))
        frame = urwid.Frame(
            urwid.AttrMap(
                urwid.ScrollBar(
                    self.listbox,
                ),
                "body",
            ),
        )
        if self.view:
            self.view.body = frame
        else:
            self.view = frame


    def show_items(self, w, parent_id):
        items = get_items(parent_id)
        body = []
        for item in items['Items']:
            button = urwid.Text(item['Name'])
            body.append(urwid.AttrMap(button, None, focus_map='reversed'))

        self.listbox = urwid.ListBox(urwid.SimpleFocusListWalker(body))
        self.view.body = urwid.AttrMap(
                urwid.ScrollBar( self.listbox,), "body",
        )



    def main(self):
        """Run the program."""
        self.loop = urwid.MainLoop(self.view, self.palette, unhandled_input=self.unhandled_input)
        self.loop.run()


    def unhandled_input(self, k):
        # update display of focus directory
        if k in {"q", "Q"}:
            raise urwid.ExitMainLoop()
        if k == 'left':
            # heck yea
            self.show_views()


def main():
    EmbyBrowser().main()


if __name__ == '__main__':
    main()
