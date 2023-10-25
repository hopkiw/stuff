#!/usr/bin/env python3

import curses
import sys

from collections import defaultdict
from curses import wrapper


WIN_REGISTER_WIDTH = 50
WIN_TEXT_WIDTH = 50
WIN_MEMORY_HEIGHT = 20
TEXT = 0x7f00
STACK = 0x5500


class Window:
    def __init__(self, win, *args, border=False):
        self.nc_w = win.derwin(*args)
        self.border = border
        self.refresh()

    def refresh(self):
        if self.border:
            self.nc_w.box()
        self.nc_w.noutrefresh()

    def addstr(self, *args):
        self.nc_w.addstr(*args)

    def addnstr(self, *args):
        self.nc_w.addnstr(*args)

    def clear(self):
        self.nc_w.erase()
        self.refresh()


class LabelWindow:
    def __init__(self, win, label, *args, border=False):
        self.label = Window(win, *args, border=border)
        self.label.addstr(1, 2, label, curses.color_pair(1))

        self.w = Window(self.label.nc_w, 4, 2)
        self.refresh()

    def refresh(self):
        self.label.refresh()
        self.w.refresh()

    def addstr(self, *args):
        self.w.addstr(*args)

    def addnstr(self, *args):
        self.w.addnstr(*args)


class ListLabelWindow(LabelWindow):
    def __init__(self, win, label, *args, border=False):
        self.label = Window(win, *args, border=border)
        self.label.addstr(1, 2, label, curses.color_pair(1))

        self.w = ListWindow(Window(self.label.nc_w, 4, 2))
        self.refresh()

    def set(self, items):
        self.w.items = items


class ListWindow:
    def __init__(self, win, items=None):
        self.w = win
        self.items = items or []
        self.selected = 0

    def refresh(self):
        self.w.clear()
        for n, item in enumerate(self.items):
            self.w.addstr(n, 0, f'{n + TEXT:#0x}: {item}')
        self.w.nc_w.chgat(self.selected, 0, -1, curses.A_REVERSE)
        self.w.refresh()

    def set(self, sel):
        if sel > len(self.items) or sel < 0:
            raise Exception('index out of range')
        self.selected = sel


class State:
    def __init__(self, registers=None, flags=None, memory=None):
        self.registers = {
                'ax': 0,
                'bx': 0,
                'cx': 0,
                'dx': 0,
                'si': 0,  # string source
                'di': 0,  # string dest
                'ip': TEXT,  # TODO: make ro
                'bp': STACK,
                'sp': STACK,
                }
        if registers:
            self.registers.update(registers)
        self.flags = {
                'zf': 0,
                }
        if flags:
            self.flags.update(flags)
        if memory:
            self.memory = memory
        else:
            self.memory = defaultdict(int)


class CPU:
    def __init__(self, instructions):
        self.instructions = instructions
        self.states = [State()]
        self.states[-1].registers['sp'] = STACK

    def prev(self):
        self.states.pop()

    def is_valid(self, operand):
        pass
    # maybe central boundary validation

    def parse_operand(self, op):
        optype = None
        try:
            val = int(op, 16)
            optype = 'i'
        except ValueError:
            pass

        if not optype:
            if len(op) == 2 and op.isalpha() and op in self.registers:
                val = op
                optype = 'r'
            elif len(op) == 4 and op[0] == '[' and op[-1] == ']':
                val = op[1:-1]
                optype = 'm'
            else:
                raise Exception('invalid operand "%s"' % op)

        return (val, optype)

    def op_ret(self, _):
        self.op_jmp('[sp]')
        self.op_add('sp,0x02')

    def op_call(self, operand):
        self.op_push('ip')
        self.op_jmp(operand)

    def op_push(self, operand):
        self.op_sub('sp,0x02')
        self.op_mov(f'[sp],{operand}')

    def op_pop(self, operand):
        self.op_mov(f'{operand},[sp]')
        self.op_add('sp,0x02')

    def op_jmp(self, operand):
        dest, desttype = self.parse_operand(operand)
        if desttype == 'r':
            dest = self.registers[dest]
            print('jmp to', hex(dest), desttype, file=sys.stderr)
        elif desttype == 'm':
            addr = self.registers[dest]
            dest1 = self.memory[addr]
            dest2 = self.memory[addr+1]
            print('jmp adds', hex(dest1), hex(dest2), file=sys.stderr)
            dest = (dest2 << 8) + dest1
            print('jmp to', hex(dest), desttype, file=sys.stderr)

        if dest & 0xff00 != TEXT:
            raise Exception('invalid jmp target')

        self.states[-1].registers['ip'] = dest

    def op_jne(self, operand):
        if self.flags['zf'] == 0:
            self.op_jmp(operand)

    def op_je(self, operand):
        if self.flags['zf'] == 1:
            self.op_jmp(operand)

    def op_cmp(self, operands):
        dest, src = operands.split(',')

        dest, desttype = self.parse_operand(dest)
        src, srctype = self.parse_operand(src)

        if srctype == 'm' and desttype == 'm':
            raise Exception('invalid source,dest pair (%s)' % (operands))
        if desttype == 'i':
            raise Exception('invalid dest operand')

        if srctype == 'r':
            val = self.registers[src]
        elif srctype == 'm':
            addr = self.registers[src]
            val = self.memory[addr]
        elif srctype == 'i':
            val = src
        else:
            raise Exception('unknown src operand type')

        if desttype == 'r':
            res = self.registers[dest] - val
        elif desttype == 'm':
            addr = self.registers[dest]
            res = self.memory[addr] - val
        else:
            raise Exception('unknown dest operand type')

        if res == 0:
            self.states[-1].flags['zf'] = 1
        else:
            self.states[-1].flags['zf'] = 0

    def op_sub(self, operands):
        dest, src = operands.split(',')

        dest, desttype = self.parse_operand(dest)
        src, srctype = self.parse_operand(src)

        if srctype == 'm' and desttype == 'm':
            raise Exception('invalid source,dest pair (%s)' % (operands))
        if desttype == 'i':
            raise Exception('invalid dest operand')

        if srctype == 'r':
            val = self.registers[src]
        elif srctype == 'm':
            addr = self.registers[src]
            val = self.memory[addr]
        elif srctype == 'i':
            val = src
        else:
            raise Exception('unknown src operand type')

        if desttype == 'r':
            newval = self.registers[dest] - val
            self.registers[dest] = newval
        elif desttype == 'm':
            addr = self.registers[dest]
            if addr & 0xff000000 != STACK:
                raise Exception('memory access out of bounds')
            newval = self.memory[addr] - val
            self.registers[addr] = newval
        else:
            raise Exception('unknown dest operand type')

        if newval == 0:
            self.flags['zf'] = 1
        else:
            self.flags['zf'] = 0

    def op_add(self, operands):
        dest, src = operands.split(',')

        dest, desttype = self.parse_operand(dest)
        src, srctype = self.parse_operand(src)

        if srctype == 'm' and desttype == 'm':
            raise Exception('invalid source,dest pair (%s)' % (operands))
        if desttype == 'i':
            raise Exception('invalid dest operand')

        if srctype == 'r':
            val = self.registers[src]
        elif srctype == 'm':
            addr = self.registers[src]
            val = self.memory[addr]
        elif srctype == 'i':
            val = src
        else:
            raise Exception('unknown src operand type')

        if desttype == 'r':
            self.registers[dest] += val
        elif desttype == 'm':
            addr = self.registers[dest]
            self.memory[addr] += val
        else:
            raise Exception('unknown dest operand type')

    def op_mov(self, operands):
        dest, src = operands.split(',')

        dest, desttype = self.parse_operand(dest)
        src, srctype = self.parse_operand(src)

        if srctype == 'm' and desttype == 'm':
            raise Exception('invalid source,dest pair (%s)' % (operands))
        if desttype == 'i':
            raise Exception('invalid dest operand')

        if srctype == 'r':
            val = self.registers[src]
        elif srctype == 'm':
            addr = self.registers[src]
            val = self.memory[addr]
        elif srctype == 'i':
            val = src
        else:
            raise Exception('unknown src operand type')

        if desttype == 'r':
            self.registers[dest] = val & 0xffff  # lower 2 bytes
        elif desttype == 'm':
            addr = self.registers[dest]
            self.memory[addr] = val & 0xff  # lower byte
            self.memory[addr+1] = val >> 0x8  # upper byte
        else:
            raise Exception('unknown dest operand type')

    def execute(self):
        self.states.append(State(self.registers, self.flags, self.memory))

        ip = self.ip
        self.states[-1].registers['ip'] += 1
        instruction = self.instructions[ip - TEXT]
        if instruction == 'nop':
            return
        op, operands = instruction.split(maxsplit=1)
        if op == 'mov':
            self.op_mov(operands)
        elif op == 'add':
            self.op_add(operands)
        elif op == 'sub':
            self.op_sub(operands)
        elif op == 'cmp':
            self.op_cmp(operands)
        elif op == 'jmp':
            self.op_jmp(operands)
        elif op == 'jne':
            self.op_jne(operands)
        elif op == 'je':
            self.op_je(operands)
        elif op == 'push':
            self.op_push(operands)
        elif op == 'pop':
            self.op_pop(operands)
        elif op == 'call':
            self.op_call(operands)
        elif op == 'ret':
            self.op_ret(operands)
        else:
            raise Exception('unsupported operation "%s"' % op)


    @property
    def registers(self):
        return self.states[-1].registers

    @property
    def flags(self):
        return self.states[-1].flags

    @property
    def memory(self):
        return self.states[-1].memory

    @property
    def ip(self):
        return self.registers['ip']

    @property
    def sp(self):
        return self.registers['sp']


def update_memory(cpu, memory_win):
    memory_win.w.nc_w.erase()
    memory_win.addstr(0, 0, '>')
    addr = cpu.sp
    for n in range(10):
        data1 = cpu.memory[addr]
        data2 = cpu.memory[addr+1]
        memory_win.addnstr(
                n, 1, f'{addr:#06x}: {data1:#06x} {data2:#06x}', 70)
        if n > 10:  # TODO calculate sizing
            break
        addr += 2
    memory_win.refresh()


def update_text(cpu, text_win):
    if not text_win.w.items:
        text_win.set(cpu.instructions)
    text_win.w.set(cpu.ip - TEXT)
    text_win.refresh()


def update_registers(cpu, register_win):
    for n, reg in enumerate(cpu.registers):
        register_win.addstr(n, 0, f'{reg}: {cpu.registers[reg]:#06x}')

    for m, flag in enumerate(cpu.flags):
        register_win.addstr(m + n + 2, 0,
                            f'{flag}: {cpu.flags[flag]}')
    register_win.refresh()


def main(stdscr):
    stdscr.clear()

    curses.init_pair(1, curses.COLOR_RED, curses.COLOR_BLACK)
    curses.curs_set(0)

    register_win = LabelWindow(
            stdscr, 'Registers',
            curses.LINES - WIN_MEMORY_HEIGHT,
            0,
            0,
            curses.COLS - WIN_REGISTER_WIDTH,
            border=True)

    text_win = ListLabelWindow(
            stdscr, 'Text',
            curses.LINES - WIN_MEMORY_HEIGHT,
            curses.COLS - WIN_REGISTER_WIDTH,
            0,
            0,
            border=True)

    memory_win = LabelWindow(
            stdscr, 'Stack',
            WIN_MEMORY_HEIGHT,
            0,
            curses.LINES - WIN_MEMORY_HEIGHT,
            0,
            border=True)

    fn = sys.argv[1] if len(sys.argv) > 1 else 'asm.txt'
    with open(fn, 'r') as fh:
        instructions = fh.read().splitlines()

    cpu = CPU(instructions)

    update_text(cpu, text_win)
    update_memory(cpu, memory_win)
    update_registers(cpu, register_win)

    refresh = False
    while True:
        curses.doupdate()
        inp = stdscr.getch()
        if inp == curses.KEY_UP or inp == ord('k'):
            if cpu.ip == TEXT:
                continue
            cpu.prev()
            refresh = True

        elif inp == curses.KEY_DOWN or inp == ord('j'):
            if cpu.ip + 1 == len(instructions) + TEXT:
                continue
            cpu.execute()
            refresh = True

        elif inp == curses.KEY_RESIZE:
            # TODO: implement resizing
            memory_win.addstr(3, 2, f'RESIZED!: {inp}')
            memory_win.refresh()

        elif inp == ord('q'):
            return

        if refresh:
            update_text(cpu, text_win)
            update_memory(cpu, memory_win)
            update_registers(cpu, register_win)
            refresh = False


if __name__ == '__main__':
    wrapper(main)

# New features:
# highlight differences step by step
# scrolling
# dynamic resizing
# Different play speeds, or even 'breakpoints'
# a print function - and a stdlib concept. DOS style, load always into
# predefined locations
# a hello, world example
#
# shift operators?
# make ops a class that handles parsing and validating their arguments?
# make memory a class?
#
#
#              highlight sub(sp,16) vs push/pop strategies
# TODO:
#      what happens with bp?
#      signed numbers
#      indirect/pointer support
#      strings, labels, functions, comments
#      necessitates output pane
#      refactor window classes
#      fix box
