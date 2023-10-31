#!/usr/bin/env python3

import curses
import curses.panel
import sys
import traceback

from collections import defaultdict
from curses import wrapper


WIN_REGISTER_WIDTH = 15
WIN_TEXT_WIDTH = 35
WIN_HEIGHT = 13

COLOR_NORMAL = 0
COLOR_RED = 1
COLOR_BLUE = 2
COLOR_YELLOW = 3

STACK = 0x7f00
TEXT = 0x5500

debug = False


def dbg(*args):
    if debug:
        print(*args, file=sys.stderr)


class Window:
    def __init__(self, title, nlines, ncols, begin_y, begin_x):
        dbg(f'Window("{title}", {nlines}, {ncols}, {begin_y}, {begin_x})')
        self.title = title

        self.frame = curses.newwin(0, 0)
        self.frame_panel = curses.panel.new_panel(self.frame)

        self.w = curses.newwin(0, 0)
        self.w_panel = curses.panel.new_panel(self.w)

        self.selected = None
        self.items = []
        self.active = False
        self.start = 0

        self.resize(nlines, ncols, begin_y, begin_x)

    def resize(self, n_lines, n_cols, start_y, start_x):
        self.frame.resize(n_lines, n_cols)
        self.frame_panel.move(start_y, start_x)
        self.frame.erase()
        self.frame.addstr(1, 2, self.title, curses.color_pair(1))

        self.w.resize(n_lines - 6, n_cols - 4)
        self.w_panel.move(start_y + 4, start_x + 2)

        self.refresh()

    def refresh(self, update_start=False):
        dbg('refresh called', self)
        if self.active:
            self.frame.attron(curses.color_pair(COLOR_RED))
            self.frame.box()
            self.frame.attrset(0)
        else:
            self.frame.box()
        self.frame.noutrefresh()

        self.w.erase()
        max_y, max_x = self.w.getmaxyx()

        # TODO: don't allow scrolling past end of items
        if update_start and self.selected is not None:
            if self.selected >= (self.start + max_y):
                self.start = self.selected - max_y
            elif self.selected <= self.start:
                self.start = self.selected

        if self.start < 0:
            self.start = 0
        elif self.start >= len(self.items):
            dbg('yep')
            # self.start = len(self.items) - 1

        # dbg('iter self.items starting at', self.start)
        for n, item in enumerate(self.items[self.start:]):
            if n >= max_y:
                break
            self.drawcolorline(n, 0, item)

        if self.selected is None:
            return

        if self.selected < self.start:
            return

        if self.selected >= self.start + max_y:
            return

        self.w.chgat(self.selected - self.start, 0, -1,
                     curses.A_REVERSE | curses.A_BOLD)

    def drawcolorline(self, y, x, line):
        self.w.move(y, x)
        for color, split in line:
            if color:
                attr = curses.color_pair(color)
            else:
                attr = curses.A_NORMAL
            self.w.addstr(split, attr)

    def setitems(self, items, refresh=True):
        self.items = items
        if refresh:
            self.refresh()

    def select(self, sel):
        if sel > len(self.items) or sel < 0:
            return
            # raise Exception('index out of range')
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
        # TODO: annoying use of asm which cpu should not know much about - will
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
            val = self.memory[addr] & 0xffff
        elif srctype == 'i':
            val = src & 0xffff
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
            val = self.memory[addr] & 0xffff
        elif srctype == 'i':
            val = src & 0xffff
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
            val = self.memory[addr] & 0xffff
        elif srctype == 'i':
            val = src & 0xffff
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
            val = self.memory[addr] & 0xffff
        elif srctype == 'i':
            val = src & 0xffff
        else:
            raise Exception('unknown src operand type')

        if desttype == 'r':
            self.registers[dest] = val
        elif desttype == 'm':
            addr = self.registers[dest]
            self.memory[addr] = val & 0xff  # lower byte
            self.memory[addr+1] = val >> 0x8  # upper byte
        else:
            raise Exception('unknown dest operand type')

    def execute(self):
        self.states.append(State(self.registers.copy(), self.flags.copy(),
                                 self.memory.copy()))

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
            labels[line[:-1]] = n
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


class RegisterWindow(Window):
    def update(self, cpu):
        registers = []
        for n, reg in enumerate(cpu.registers):
            registers.append([(COLOR_BLUE, f'{reg}'),
                              (COLOR_NORMAL, f' {cpu.registers[reg]:#06x}')])

        for m, flag in enumerate(cpu.flags):
            registers.append([(COLOR_NORMAL, f'{flag}: {cpu.flags[flag]}')])

        self.setitems(registers)


class MemoryWindow(Window):
    def update(self, cpu):
        memory = []
        addr = self.start + TEXT

        dbg('will draw', addr, 'through', addr+14)
        dbg('memory is', cpu.memory)
        for _ in range(14):
            data1 = cpu.memory[addr]
            data2 = cpu.memory[addr+1]
            memory.append([(COLOR_BLUE, f'{addr + TEXT:#06x}'),
                           (COLOR_NORMAL, f' {data1:#04x} {data2:#04x}')])
            if addr == cpu.sp:
                memory[-1].append((COLOR_NORMAL, '  <-'))
            addr += 2

        dbg('update: setting items to:', memory)
        self.setitems(memory)


class TextWindow(Window):
    def update(self, cpu, program, follow=False):
        formatted = []
        for addr, i in enumerate(program):
            formatted.append([(COLOR_BLUE, f'{addr + TEXT:#06x}'),
                              (COLOR_NORMAL, f' {i}')])
        for label, addr in self.labels.items():
            formatted[addr].extend([(COLOR_YELLOW, f' # {label}')])
        self.setitems(formatted, False)
        self.select(cpu.ip - TEXT)
        self.refresh(follow)


def main(stdscr):
    global debug

    err_win_panel = None

    stdscr.clear()

    curses.init_pair(COLOR_RED, curses.COLOR_RED, curses.COLOR_BLACK)
    curses.init_pair(COLOR_BLUE, curses.COLOR_BLUE, curses.COLOR_BLACK)
    curses.init_pair(COLOR_YELLOW, curses.COLOR_YELLOW, curses.COLOR_BLACK)
    curses.curs_set(0)  # hide cursor

    minwidth = WIN_REGISTER_WIDTH + WIN_TEXT_WIDTH
    minheight = WIN_HEIGHT * 2

    if curses.COLS < minwidth or curses.LINES < minheight:
        raise Exception('window too small - %dx%d is less than minimum %dx%d'
                        % (curses.COLS, curses.LINES, minwidth, minheight))

    max_y, max_x = stdscr.getmaxyx()
    lower_win_height = max(WIN_HEIGHT, max_y // 3)

    # Memory window: entire lower region
    memory_win = MemoryWindow(
            'Stack',
            lower_win_height,
            curses.COLS,
            curses.LINES - lower_win_height,
            0)

    upper_win_width = max(WIN_REGISTER_WIDTH, max_x // 3)

    # Text window: left 2/3rds of upper region.
    text_win = TextWindow(
            'Text',
            curses.LINES - lower_win_height,
            curses.COLS - upper_win_width,
            0,
            0)

    # Register window: right 1/3rd of upper region.
    register_win = RegisterWindow(
            'Registers',
            curses.LINES - lower_win_height,
            curses.COLS - (curses.COLS - upper_win_width),
            0,
            curses.COLS - upper_win_width)

    fn = sys.argv[1] if len(sys.argv) > 1 else 'asm.txt'
    with open(fn, 'r') as fh:
        lines = fh.read().splitlines()

    instructions, labels = parse_program(lines)
    program = parse_instructions(instructions, labels)
    text_win.labels = labels

    cpu = CPU(program, labels['_start'])

    refresh = True
    parse_mode = False
    windows = [text_win, memory_win]
    selwin = 0
    follow = True
    while True:
        if refresh:
            windows[selwin].active = True
            windows[selwin-1].active = False

            text_win.update(cpu, program if parse_mode else instructions,
                            follow)
            debug = True
            memory_win.update(cpu)
            debug = False
            register_win.update(cpu)
            refresh = False

        curses.panel.update_panels()
        curses.doupdate()

        inp = stdscr.getch()
        if inp == curses.KEY_UP or inp == ord('k'):
            if windows[selwin] != text_win:
                continue
            if err_win_panel is not None:
                continue
            cpu.prev()
            follow = True
            refresh = True

        elif inp == curses.KEY_DOWN or inp == ord('j'):
            if windows[selwin] != text_win:
                continue
            if err_win_panel is not None:
                continue
            if cpu.ip + 1 == len(cpu.instructions) + TEXT:
                continue
            cpu.execute()
            follow = True
            refresh = True

        elif inp == ord('J'):
            if err_win_panel is not None:
                continue
            if windows[selwin].start >= len(windows[selwin].items):
                continue
            windows[selwin].start += 1
            follow = False
            refresh = True

        elif inp == ord('K'):
            if err_win_panel is not None:
                continue
            if windows[selwin].start <= 0:
                continue
            windows[selwin].start -= 1
            follow = False
            refresh = True

        elif inp == ord('\t'):
            # Cycle active window
            if selwin:
                selwin = 0
            else:
                selwin = 1
            refresh = True

        elif inp == curses.KEY_RESIZE:
            stdscr.erase()
            stdscr.noutrefresh()

            max_y, max_x = stdscr.getmaxyx()
            lower_win_height = max(WIN_HEIGHT, max_y // 3)
            upper_win_width = max(WIN_REGISTER_WIDTH, max_x // 3)
            dbg('stdscr is now', (max_y, max_x))

            if (
                    max_y < (2 * WIN_HEIGHT)
                    or max_x < (WIN_TEXT_WIDTH + WIN_REGISTER_WIDTH)):
                if not err_win_panel:
                    err_win = curses.newwin(0, 0)
                    err_win_panel = curses.panel.new_panel(err_win)
                    err_win.box('!', '!')
                    err_win.addstr(max_y // 2, max_x // 2 - 10,
                                   'WINDOW TOO SMALL')
                    err_win.noutrefresh()
                continue

            # resize each window
            try:
                memory_win.resize(
                        lower_win_height,
                        max_x,
                        max_y - lower_win_height,
                        0)
                text_win.resize(
                        max_y - lower_win_height,
                        max_x - upper_win_width,
                        0,
                        0)
                register_win.resize(
                        max_y - lower_win_height,
                        upper_win_width,
                        0,
                        max_x - upper_win_width)

                refresh = True
                err_win_panel = None
            except Exception as e:
                dbg('exception during resize:', e)
                traceback.print_exception(e, file=sys.stderr)
                if not err_win_panel:
                    err_win = curses.newwin(0, 0)
                    err_win_panel = curses.panel.new_panel(err_win)
                    err_win.addstr(max_y // 2, max_x // 2, 'NCURSES ERROR')
                    err_win.noutrefresh()

        elif inp == ord('p'):
            parse_mode = not parse_mode
            dbg('parse_mode:', parse_mode)
            refresh = True

        elif inp == ord('q'):
            dbg('quit')
            return

        else:
            dbg('unknown ch', inp)
            dbg('tab is', inp == 9)


if __name__ == '__main__':
    wrapper(main)
    print('Done.')

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
#      make program with real value
#      state-independent scrolling and memory scrolling
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
# yes, we will need panel after all
#
# resizing: display error if screen is too small
# switch layouts?
# minimimum height & width for each
# let's start there
#
# bold window names
# colored outputs
#
# registers right 1/3rd or 1/4th, with max and min; dont expand beyond max,
# error if below min
#
# if impossible_to_show:
#   while loop only handles q and [resize]
