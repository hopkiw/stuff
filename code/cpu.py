#!/usr/bin/env python3

import curses
import sys
from curses import wrapper


WIN_REGISTER_WIDTH = 50
WIN_TEXT_WIDTH = 50
WIN_MEMORY_HEIGHT = 20


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

    def add(self, item):
        if hasattr(item, '__getitem__'):
            self.w.items.extend(item)
        else:
            self.w.items.append(item)


class ListWindow:
    def __init__(self, win, items=None):
        self.w = win
        self.items = items or []
        self.selected = 0

    def refresh(self):
        self.w.clear()
        for n, item in enumerate(self.items):
            self.w.addstr(n, 0, f'{n:#06x}: {item}')
        self.w.nc_w.chgat(self.selected, 0, -1, curses.A_REVERSE)
        self.w.refresh()

    def set(self, sel):
        if sel > len(self.items):
            raise Exception('index out of range')
        elif sel < 0:
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
                'ip': 0,
                'bp': 0,
                'sp': 0,
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
            self.memory = [0] * 1024


class CPU:
    def __init__(self, instructions):
        self.instructions = instructions
        self.states = [State()]

#    def next(self):
#        # Generate on-demand to allow for future limitations in undo buffer
#        if self.pos + 1 == len(self.states):
#            state = self.states[self.pos]
#            newstate = State(state.registers, state.flags, state.memory)
#            self.states.append(newstate)
#        self.pos += 1
#
#    def prev(self):
#        if self.pos == 0:
#            return
#        self.pos -= 1

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

    def jmp(self, operand):
        dest, desttype = self.parse_operand(operand)
        if desttype == 'r':
            dest = self.registers[dest]

        if dest >= len(self.instructions):
            raise Exception('invalid jmp target')

        self.states[-1].registers['ip'] = dest

    def jne(self, operand):
        if self.flags['zf'] == 0:
            self.jmp(operand)

    def je(self, operand):
        if self.flags['zf'] == 1:
            self.jmp(operand)

    def cmp(self, operands):
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
            if addr <= len(self.instructions):
                raise Exception('memory access out of bounds')
            res = self.memory[addr] - val
        else:
            raise Exception('unknown dest operand type')

        if res == 0:
            self.states[-1].flags['zf'] = 1
        else:
            self.states[-1].flags['zf'] = 0

    def sub(self, operands):
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
            if addr <= len(self.instructions):
                raise Exception('memory access out of bounds')
            newval = self.memory[addr] - val
            self.registers[addr] = newval
        else:
            raise Exception('unknown dest operand type')

        if newval == 0:
            self.flags['zf'] = 1
        else:
            self.flags['zf'] = 0

    def add(self, operands):
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
            if addr <= len(self.instructions):
                raise Exception('memory access out of bounds')
            self.memory[addr] += val
        else:
            raise Exception('unknown dest operand type')

    def mov(self, operands):
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
            self.registers[dest] = val
        elif desttype == 'm':
            addr = self.registers[dest]
            if addr <= len(self.instructions):
                raise Exception('memory access out of bounds')
            self.memory[addr] = val
        else:
            raise Exception('unknown dest operand type')

    def execute(self):
        instruction = self.instructions[self.ip]
        op, operands = instruction.split(maxsplit=1)
        ip = self.ip
        if op == 'mov':
            self.mov(operands)
        elif op == 'add':
            self.add(operands)
        elif op == 'sub':
            self.sub(operands)
        elif op == 'cmp':
            self.cmp(operands)
        elif op == 'jmp':
            self.jmp(operands)
        elif op == 'jne':
            self.jne(operands)
        elif op == 'je':
            self.je(operands)
        elif op == 'nop':
            return
        else:
            raise Exception('invalid operation "%s"' % op)

        if ip == self.ip:
            self.states[-1].registers['ip'] += 1

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


def main(stdscr):
    # Clear screen
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

    text_win.add(instructions)
    text_win.refresh()

    cpu = CPU(instructions)

    for n, reg in enumerate(cpu.registers):
        register_win.addstr(n, 0, f'{reg}: {cpu.registers[reg]:#06x}')

    for m, flag in enumerate(cpu.flags):
        register_win.addstr(m + n + 2, 0,
                            f'{flag}: {cpu.flags[flag]}')

    register_win.refresh()

#    with open('sample.txt', 'r') as fh:
#        lines = fh.read().splitlines()
#
#    for n, line in enumerate(lines):
#        line = f'{n + len(instructions):#06x}: {line}'
#        memory_win.addstr(n, 0, line)
#        if n > 10:
#            break
#
#    memory_win.refresh()

    curses.doupdate()

    while True:
        inp = stdscr.getch()
        if inp == curses.KEY_UP or inp == ord('k'):
            pass
#            if cpu.pos == 0:
#                continue
#            # text_win.w.prev()
#            cpu.prev()
#            text_win.w.selected = cpu.pos
#            text_win.refresh()
#            for n, reg in enumerate(cpu.registers):
#                register_win.addstr(n, 0, f'{reg}: {cpu.registers[reg]:#06x}')
#
#            for m, flag in enumerate(cpu.flags):
#                register_win.addstr(m + n + 2, 0,
#                                    f'{flag}: {cpu.flags[flag]}')
#                register_win.refresh()
        elif inp == curses.KEY_DOWN or inp == ord('j'):
            if cpu.ip + 1 == len(instructions):
                continue
            cpu.execute()
            text_win.w.selected = cpu.ip
            text_win.refresh()
            for n, reg in enumerate(cpu.registers):
                register_win.addstr(n, 0, f'{reg}: {cpu.registers[reg]:#06x}')

            for m, flag in enumerate(cpu.flags):
                register_win.addstr(m + n + 2, 0,
                                    f'{flag}: {cpu.flags[flag]}')
                register_win.refresh()

            for n, val in enumerate(cpu.memory):
                data = cpu.memory[n + len(instructions)]
                memory_win.addnstr(
                        n, 0, f'{n + len(instructions):#06x}: {data:#06x}', 70)
                if n > 10:
                    break

            memory_win.addnstr(0, 0, str(cpu.ip), 70)
            memory_win.refresh()
        elif inp == curses.KEY_RESIZE:
            memory_win.addstr(3, 2, f'RESIZED!: {inp}')
            memory_win.refresh()
        elif inp == ord('q'):
            return

        # register_win.addstr(m + n + 4, 0, f'{cpu.registers}')
        register_win.refresh()
        curses.doupdate()


if __name__ == '__main__':
    res = wrapper(main)
    print('got res:', res)

# Features:
# Display asm code with addresses in a scrollable window
# Implement c-calling convention, mov, lea, add, sub, push, pop, call, return,
# jne, jle, jg, cmp, etc.
# maybe some make-believe print function
#
# progress to a fd that you copy byte by byte to. a 'print' function would copy
# from memory to the stdout fd until reaching \0. this means a byte by byte
# comparison which may or may not accept a memory address. if it doesn't,
# each byte needs to be moved to a register first
#
# if we accept input as an editor and input to a program, we need to make them
# clear. maybe a 'shell' that can 'edit' files
#
# Read a 'program' in  asm from a file
# Enter a 'program' manually
# Step through each fetch-execute cycle
# Ctrl-Z, Ctrl-R (Undo/Redo)
# Visual state of memory and registers at all times
# Different play speeds, or even 'breakpoints'
# Possible to randomize address space? you'd need labels in asm right, so the
# compiler could replace them with real addresses. in fact that's an asm vs
# machine code
#
# functions return in eax (single return)
# before you call you put arguments on the stack
#
# use intel asm syntax
# highlight differences step by step
# scrolling
#
# assembly
# mov r/m8, r8  # move r8 to r/m8
# mov r8, r/m8  # move r/m8 to r8
# mov r/m8, imm8  # move imm8 to r/m8
#
# memory window has two modes, depending on size of screen, showing more at a
# time
#
# read every byte (two hex chars) and pass to chr; or '.'
#
# addr: b1 b2 b3 b4 b5 b6 b7 b8 .ELF....
#
# restrict sizes
# list of objects or bytestring?
