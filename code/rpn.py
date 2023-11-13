#!/usr/bin/env python3

import curses
from curses.textpad import Textbox
import sys


class GrubLineEditor(Textbox):
    def __init__(self, screen, startx, starty, line=""):
        screen.addstr(startx, starty, "> ")
        screen.noutrefresh()
        win = curses.newwin(1, 74, startx, starty + 2)
        curses.textpad.Textbox.__init__(self, win)

        self.line = list(line)
        self.pos = len(line)
        self.cancelled = False
        self.show_text()

    def show_text(self):
        """Show the text.  One of our advantages over standard textboxes
        is that we can handle lines longer than the window."""

        self.win.erase()
        p = self.pos
        off = 0
        while p > 70:
            p -= 55
            off += 55

        line = self.line[off:off+70]
        self.win.addstr(0, 0, "".join(line))
        if self.pos > 70:
            self.win.addch(0, 0, curses.ACS_LARROW)

        self.win.move(0, p)

    def do_command(self, ch):
        # we handle escape as well as moving the line around, so have
        # to override some of the default handling

        self.lastcmd = ch
        if ch == 27:  # esc
            self.cancelled = True
            return 0
        elif curses.ascii.isprint(ch):
            self.line.insert(self.pos, chr(ch))
            self.pos += 1
        elif ch == curses.ascii.SOH:  # ^a
            self.pos = 0
        elif ch in (curses.ascii.STX, curses.KEY_LEFT):
            if self.pos > 0:
                self.pos -= 1
        elif ch in (curses.ascii.BS, curses.KEY_BACKSPACE):
            if self.pos > 0:
                self.pos -= 1
                if self.pos < len(self.line):
                    self.line.pop(self.pos)
        elif ch == curses.ascii.EOT:                           # ^d
            if self.pos < len(self.line):
                self.line.pop(self.pos)
        elif ch == curses.ascii.ENQ:                           # ^e
            self.pos = len(self.line)
        elif ch in (curses.ascii.ACK, curses.KEY_RIGHT):
            if self.pos < len(self.line):
                self.pos += 1
        elif ch == curses.ascii.VT:                            # ^k
            self.line = self.line[:self.pos]
        else:
            return curses.textpad.Textbox.do_command(self, ch)
        self.show_text()
        return 1

    def edit(self):
        curses.doupdate()
        curses.textpad.Textbox.edit(self)
        if self.cancelled:
            return None
        return "".join(self.line)


def main(stdscr):
    stdscr.clear()

    curses.init_pair(1, curses.COLOR_RED, curses.COLOR_BLACK)
    curses.init_pair(2, curses.COLOR_BLUE, curses.COLOR_BLACK)
    curses.init_pair(3, curses.COLOR_YELLOW, curses.COLOR_BLACK)
    curses.curs_set(0)

    max_y, max_x = stdscr.getmaxyx()

    textwin = curses.newwin(20, 25, 2, 2)
    textwin.box()
    # textwin.hline(max_y - 4, 0, 0, max_x)
    textwin.addstr(1, 2, 'hi', curses.color_pair(1))
    textwin.noutrefresh()

    stack = []
    while True:
        for n, line in enumerate(stack):
            print('line is', str(line), file=sys.stderr)
            textwin.addstr(2 + n, 2, str(line), curses.color_pair(1))
        curses.doupdate()
        res = textwin.getch()
        if res == ord('q'):
            break
        elif res == 10:
            t = GrubLineEditor(stdscr, max_y-3, 2)
            curses.curs_set(1)
            res = t.edit()
            curses.curs_set(0)
            print('got edit', str(res), file=sys.stderr)
            if res == 'add':
                arg1 = stack.pop()
                arg2 = stack.pop()
                stack.append(arg1 + arg2)
            elif res == 'sub':
                pass
            elif res == 'mul':
                pass
            elif res == 'div':
                pass
            elif res == 'pow':
                pass
            elif res == 'mod':
                pass
            elif res == 'sqrt':
                pass
            elif res == 'clr':
                pass
            elif res == 'dup':
                pass
            elif res == 'swap':
                pass
            elif res == 'rot':
                pass
            else:
                try:
                    res = int(res, 0)
                except ValueError:
                    pass

                stack.append(res)

        else:
            print('unk key', res, file=sys.stderr)


if __name__ == '__main__':
    curses.wrapper(main)

# TODO:
#   * need to decide on UI
#   * animating/state changes
#
#   UI:
#
#   3
#   32
#   181
#   0
#   2
#
#   text entry area: always present, or popping up as needed?
#   default view is...?
#
#   accomplish complicated ((2 + 1) * (13 - ( 4 / 2))) + 12
#   via correct stack ordering
#
#   dup, add, sub, mul, pow, etc.
#   if op in ops:
#     opfunc(op)
#   elif isint(op):
#     push(int(op))
#   else:
#     push(str(op))
