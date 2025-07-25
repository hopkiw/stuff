#!/usr/bin/env python3

import urwid
from emby_tool import get_views, get_items


def menu(title, choices_):
    body = [urwid.Text(title), urwid.Divider()]
    for c in choices_:
        button = urwid.Button(c['Name'])
        urwid.connect_signal(button, 'click', item_chosen, c)
        body.append(urwid.AttrMap(button, None, focus_map='reversed'))

    return urwid.ListBox(urwid.SimpleFocusListWalker(body))


def item_chosen(button, choice):
    body = []
    items = get_items(choice['Id'])
    for item in items['Items']:
        button = urwid.Button(item['Name'])
        body.append(urwid.AttrMap(button, None, focus_map='reversed'))
    boxlist = urwid.ListBox(urwid.SimpleFocusListWalker(body))
    main_win.original_widget = urwid.Padding(boxlist, left=2, right=2)


main_win = None


def exit_program(button):
    raise urwid.ExitMainLoop()


def main():
    global main_win
    # choices = ["Chapman", "Cleese", "Gilliam", "Idle", "Jones", "Palin"]
    views = get_views()
    choices = []
    for view in views['Items']:
        entry = {'Name': view['Name'], 'Id': view['Id']}
        choices.append(entry)
    boxlist = menu('Views', choices)
    main_win = urwid.Padding(boxlist, left=2, right=2)
    top = urwid.Overlay(
        main_win,
        urwid.SolidFill("\N{MEDIUM SHADE}"),
        align=urwid.CENTER,
        width=(urwid.RELATIVE, 60),
        valign=urwid.MIDDLE,
        height=(urwid.RELATIVE, 60),
        min_width=20,
        min_height=9,
    )
    urwid.MainLoop(top, palette=[("reversed", "standout", "")]).run()


if __name__ == '__main__':
    main()
