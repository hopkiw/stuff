#!/usr/bin/env python3

import curses
from curses.textpad import Textbox
import math
import sys


class GrubLineEditor(Textbox):
    def __init__(self, starty, startx, line=""):
        win = curses.newwin(1, 74, starty, startx)
        curses.textpad.Textbox.__init__(self, win)

        self.line = list(line)
        self.pos = len(line)
        self.cancelled = False
        self.show_text()

    def show_text(self):
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
    textwin.noutrefresh()
    editwin = curses.newwin(3, max_x - 1, 24, 0)
    editwin.noutrefresh()

    while True:
        textwin.erase()
        stack_copy = stack.copy()
        for n in range(15):
            if n >= len(stack):
                line = ''
            else:
                line = stack_copy.pop()
            line = f'{n:02}: {line}'
            textwin.addstr(2 + n, 2, line, curses.color_pair(1))
        textwin.noutrefresh()
        textwin.box()

        curses.doupdate()

        res = textwin.getch()
        if res == 10:
            editwin.addstr(1, 1, '> ')
            editwin.noutrefresh()
            t = GrubLineEditor(25, 3)
            curses.curs_set(1)
            res = t.edit()
            curses.curs_set(0)
            editwin.erase()
            editwin.noutrefresh()
            if res:
                try:
                    _exec(res)
                except IndexError:
                    raise Exception('stack empty')

        else:
            print('unk key', res, file=sys.stderr)


stack = []


def _exec(res):
    if res == 'add':
        arg1 = stack.pop()
        arg2 = stack.pop()
        res = arg2 + arg1
        if int(res) == res:
            res = int(res)
        stack.append(res)
    elif res == 'sub':
        arg1 = stack.pop()
        arg2 = stack.pop()
        res = arg2 - arg1
        if int(res) == res:
            res = int(res)
        stack.append(res)
    elif res == 'mul':
        arg1 = stack.pop()
        arg2 = stack.pop()
        res = arg2 * arg1
        if int(res) == res:
            res = int(res)
        stack.append(res)
    elif res == 'div':
        arg1 = stack.pop()
        arg2 = stack.pop()
        res = arg2 / arg1
        if int(res) == res:
            res = int(res)
        stack.append(res)
    elif res == 'pow':
        arg1 = stack.pop()
        arg2 = stack.pop()
        res = arg2 ** arg1
        if int(res) == res:
            res = int(res)
        stack.append(res)
    elif res == 'mod':
        arg1 = stack.pop()
        arg2 = stack.pop()
        res = arg2 % arg1
        if int(res) == res:
            res = int(res)
        stack.append(res)
    elif res == 'sqrt':
        arg1 = stack.pop()
        stack.append(math.sqrt(arg1))
    elif res == 'shr':
        arg1 = stack.pop()
        arg2 = stack.pop()
        return arg1 >> arg2
    elif res == 'clr':
        stack.clear()
    elif res == 'dup':
        arg1 = stack.pop()
        stack.append(arg1)
        stack.append(arg1)
    elif res == 'swap':
        arg1 = stack.pop()
        arg2 = stack.pop()
        stack.append(arg1)
        stack.append(arg2)
    elif res == 'quit':
        return 'quit'
    else:
        try:
            res = float(res)
            if res.is_integer():
                res = int(res)
            stack.append(res)
        except ValueError:
            res = int(res, 0)
            stack.append(res)


if __name__ == '__main__':
    try:
        curses.wrapper(main)
    except Exception as e:
        print(e)
