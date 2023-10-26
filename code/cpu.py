#!/usr/bin/env python3

import curses
import sys

from collections import defaultdict
from curses import wrapper


WIN_REGISTER_WIDTH = 15
WIN_TEXT_WIDTH = 35
WIN_HEIGHT = 14

TEXT = 0x7f00
STACK = 0x5500


class Window:
    def __init__(self, parent, label, *args, border=True):
        self.label = parent.derwin(*args)
        self.w = self.label.derwin(4, 2)

        self.label.addstr(1, 2, label, curses.color_pair(1))
        self.selected = None
        self.items = []
        self.border = border
        self.refresh()

    def refresh(self):
        self.w.erase()
        for n, item in enumerate(self.items):
            self.w.addstr(n, 0, item)
        if self.selected is not None:
            self.w.chgat(self.selected, 0, -1, curses.A_REVERSE)
        if self.border:
            self.label.box()
        self.w.noutrefresh()

    def addstr(self, *args):
        self.w.addstr(*args)

    def addnstr(self, *args):
        self.w.addnstr(*args)

    def setitems(self, items):
        self.items = items

    def select(self, sel):
        if sel > len(self.items) or sel < 0:
            raise Exception('index out of range')
        self.selected = sel


class State:
    def __init__(self, registers=None, flags=None, memory=None):
        self.registers = {reg: 0 for reg in CPU._registers}
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
    _registers = ['ax', 'bx', 'cx', 'dx', 'si', 'di', 'ip', 'bp', 'sp']

    def __init__(self, program, offset=0):
        self.instructions = program
        self.states = [State()]
        self.states[-1].registers['sp'] = STACK
        self.states[-1].registers['bp'] = STACK

        self._fix_offsets()
        self.states[-1].registers['ip'] = offset + TEXT

    def _fix_offsets(self):
        for i in range(len(self.instructions)):
            op, operands = self.instructions[i]
            new_operands = []
            for operand in operands:
                operand, optype = operand
                if optype == 'l':
                    operand = operand + TEXT
                    optype = 'i'
                new_operands.append((operand, optype))
            self.instructions[i] = (op, new_operands)

    def prev(self):
        if len(self.states) > 1:
            self.states.pop()

    def op_ret(self, _):
        # annoying use of asm which cpu should not know much about - will
        # replace with operand classes
        self.op_jmp(parse_operands('[sp]'))
        self.op_add(parse_operands('sp,0x02'))

    def op_call(self, operand):
        self.op_push(parse_operands('ip'))
        self.op_jmp(operand)

    def op_push(self, operand):
        self.op_sub(parse_operands('sp,0x02'))
        stack = parse_operands('[sp]')
        self.op_mov((stack[0], operand[0]))

    def op_pop(self, operand):
        stack = parse_operands('[sp]')
        self.op_mov((operand[0], stack[0]))
        self.op_add(parse_operands('sp,0x02'))

    def op_jmp(self, operand):
        dest, desttype = operand[0]
        if desttype == 'r':
            dest = self.registers[dest]
        elif desttype == 'm':
            addr = self.registers[dest]
            dest1 = self.memory[addr]
            dest2 = self.memory[addr+1]
            dest = (dest2 << 8) + dest1

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
        dest, src = operands

        dest, desttype = dest
        src, srctype = src

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
        dest, src = operands

        dest, desttype = dest
        src, srctype = src

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
        dest, src = operands

        dest, desttype = dest
        src, srctype = src

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
        dest, src = operands

        dest, desttype = dest
        src, srctype = src

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
        op, operands = self.instructions[ip - TEXT]
        # TODO: membership list vs elif chain
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


def parse_program(program):
    instructions = []
    labels = {}
    n = 0
    for line in program:
        line = line.split(';')[0].strip()
        if not line:
            continue

        if line.endswith(':'):
            labels[line[:-1]] = n  # + TEXT
        else:
            instructions.append(line)
            n += 1

    if '_start' not in labels:
        labels['_start'] = TEXT

    return instructions, labels


def parse_instructions(instructions, labels):
    new_instructions = []
    for n, instruction in enumerate(instructions):
        instruction = instruction.split(maxsplit=1)
        if len(instruction) < 2:
            instruction = instruction[0]
            if instruction not in ['nop', 'ret']:
                raise Exception('invalid instruction: %s requires operands'
                                % instruction)
            new_instructions.append((instruction, tuple()))
        else:
            op, operands = instruction
            operands = parse_operands(operands, labels)
            new_instructions.append((op, operands))

    return new_instructions


def parse_operands(operands, labels=None):
    if not labels:
        labels = tuple()

    operand_pairs = []
    for op in operands.split(','):
        optype = None
        op = op.strip()
        if op in labels:
            val = labels[op]
            optype = 'l'
        elif len(op) == 2 and op.isalpha() and op in CPU._registers:
            val = op
            optype = 'r'
        elif len(op) == 4 and op[0] == '[' and op[-1] == ']':
            val = op[1:-1]
            optype = 'm'
        else:
            try:
                val = int(op, 16)
                optype = 'i'
            except ValueError:
                raise Exception('invalid operand "%s"' % op)

        operand_pairs.append((val, optype))

    return operand_pairs


def update_memory(cpu, memory_win):
    memory_win.w.erase()
    memory_win.addstr(0, 0, '>')
    addr = cpu.sp
    memory = []
    for n in range(10):
        data1 = cpu.memory[addr]
        data2 = cpu.memory[addr+1]
        memory.append(f'{addr:#06x}: {data1:#06x} {data2:#06x}')
        addr += 2
    memory_win.setitems(memory)
    memory_win.refresh()


def update_text(cpu, text_win, program, labels):
    if not text_win.items:
        formatted = []
        for addr, i in enumerate(program):
            formatted.append(f'{addr + TEXT:#06x}: {i}')
        text_win.setitems(formatted)
        for label, addr in labels.items():
            orig = text_win.items[addr]
            text_win.items[addr] = f'{orig}  # {label}'
    text_win.select(cpu.ip - TEXT)
    text_win.refresh()


def update_registers(cpu, register_win):
    registers = []
    for n, reg in enumerate(cpu.registers):
        registers.append(f'{reg}: {cpu.registers[reg]:#06x}')

    for m, flag in enumerate(cpu.flags):
        registers.append(f'{flag}: {cpu.flags[flag]}')

    register_win.setitems(registers)
    register_win.refresh()


def main(stdscr):
    stdscr.clear()

    curses.init_pair(1, curses.COLOR_RED, curses.COLOR_BLACK)
    curses.curs_set(0)  # hide cursor

    minwidth = WIN_REGISTER_WIDTH + WIN_TEXT_WIDTH
    minheight = WIN_HEIGHT * 2
    if curses.COLS < minwidth or curses.LINES < minheight:
        raise Exception('window too small - %dx%d is less than minimum %dx%d'
                        % (curses.COLS, curses.LINES, minwidth, minheight))

    register_win = Window(
            stdscr, 'Registers',
            curses.LINES - WIN_HEIGHT,
            0,
            0,
            curses.COLS - WIN_REGISTER_WIDTH,
            border=True)

    text_win = Window(
            stdscr, 'Text',
            curses.LINES - WIN_HEIGHT,
            curses.COLS - WIN_REGISTER_WIDTH,
            0,
            0,
            border=True)

    memory_win = Window(
            stdscr, 'Stack',
            WIN_HEIGHT,
            0,
            curses.LINES - WIN_HEIGHT,
            0,
            border=True)

    fn = sys.argv[1] if len(sys.argv) > 1 else 'asm.txt'
    with open(fn, 'r') as fh:
        lines = fh.read().splitlines()

    instructions, labels = parse_program(lines)
    program = parse_instructions(instructions, labels)

    cpu = CPU(program, labels['_start'])

    update_text(cpu, text_win, instructions, labels)
    update_memory(cpu, memory_win)
    update_registers(cpu, register_win)

    refresh = False
    while True:
        curses.doupdate()
        inp = stdscr.getch()
        if inp == curses.KEY_UP or inp == ord('k'):
            cpu.prev()
            refresh = True

        elif inp == curses.KEY_DOWN or inp == ord('j'):
            if cpu.ip + 1 == len(cpu.instructions) + TEXT:
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
            update_text(cpu, text_win, program, labels)
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
#      strings / initialized data
#      in/out instructions (necessitates I/O pane)
#      fix box
#      make program with real value
#
# ---
#
# TextWindow (subclass LabelWindow with ref to cpu, reflects cpu.instructions)
# MemoryWindow (subclass LabelWindow with ref to cpu, reflects cpu.memory)
# RegisterWindow (subclass LabelWindow with ref to cpu, reflects cpu.registers)
# use super() as needed
#
# code outside cpu shouldn't reference TEXT/STACK ?
#
# custom exception types and tests
#
# in can only write to eax; can only read a port number from an immediate or
# from dx
#
# show labeled / swap to real addresses
# show comments
# swap between parsed and original source code representations
#
# swaps may mean using nc.panel?
# yes, classes for instructions / operators / operands
#
# resizing: display error if screen is too small
# switch layouts?
# minimimum height & width for each
# let's start there
#
# bold window names
# colored outputs
