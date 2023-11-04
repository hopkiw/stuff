#!/usr/bin/env python3

from collections import defaultdict


WIN_REGISTER_WIDTH = 15
WIN_TEXT_WIDTH = 35
WIN_HEIGHT = 13

COLOR_NORMAL = 0
COLOR_RED = 1
COLOR_BLUE = 2
COLOR_YELLOW = 3

STACK = 0x7f00
TEXT = 0x5500
DATA = 0x6b00


def parse_operands(operands, text_labels=None, data_labels=None):
    if not text_labels:
        text_labels = tuple()
    if not data_labels:
        data_labels = tuple()

    operand_pairs = []
    for op in operands.split(','):
        optype = None
        op = op.strip()
        if op in text_labels:
            val = text_labels[op]
            optype = 'tl'
        elif op in data_labels:
            val = data_labels[op]
            optype = 'dl'
        elif len(op) == 2 and op.isalpha() and op in CPU._registers:
            val = op
            optype = 'r'
        elif op[0] == '[' and op[-1] == ']':
            val = op[1:-1]
            # do some validations
            if len(val) > 2:
                aop, num = val[2], val[3:]
                if aop not in ('+', '-'):
                    raise Exception('invalid operand addressing "%s"' % op)
                try:
                    int(num, 16)
                except ValueError:
                    raise Exception('invalid operand addressing "%s"' % op)
            optype = 'm'
        else:
            try:
                val = int(op, 16)
                optype = 'i'
            except ValueError:
                raise Exception('invalid operand "%s"' % op)

        operand_pairs.append((val, optype))

    return operand_pairs


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

    def __init__(self, program, offset=0, data=None):
        self.instructions = program
        self.states = [State()]
        self.states[-1].registers['sp'] = STACK
        self.states[-1].registers['bp'] = 0x1

        self._fix_offsets()
        self.states[-1].registers['ip'] = offset + TEXT

        self._load(data)
        self.data = data  # to detect if data is present

    def _load(self, data):
        for addr in data:
            self.memory[addr + DATA] = data[addr]

    def _fix_offsets(self):
        for i in range(len(self.instructions)):
            op, operands = self.instructions[i]
            new_operands = []
            for operand in operands:
                operand, optype = operand
                if optype == 'tl':
                    operand = operand + TEXT
                    optype = 'i'
                elif optype == 'dl':
                    operand = operand + DATA
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
            addr = self._memory_operand(dest)
            dest1 = self.memory[addr]
            dest2 = self.memory[addr+1]
            dest = (dest2 << 8) + dest1

        if dest & 0xff00 != TEXT:
            raise Exception('invalid jmp target %x' % dest)

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
            addr = self._memory_operand(src)
            val = self.memory[addr] & 0xffff
        elif srctype == 'i':
            val = src & 0xffff
        else:
            raise Exception('unknown src operand type %s' % srctype)

        if desttype == 'r':
            res = self.registers[dest] - val
        elif desttype == 'm':
            addr = self._memory_operand(dest)
            res = self.memory[addr] - val
        else:
            raise Exception('unknown dest operand type')

        if res == 0:
            self.states[-1].flags['zf'] = 1
        else:
            self.states[-1].flags['zf'] = 0

    def _memory_operand(self, op):
        addr = self.registers[op[:2]]
        if op[2:]:
            o, num = op[2], op[3:]
            num = int(num, 16)
            if o == '+':
                addr = addr + num
            elif o == '-':
                addr = addr - num

        return addr

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
            addr = self._memory_operand(src)
            val = self.memory[addr] & 0xffff
        elif srctype == 'i':
            val = src & 0xffff
        else:
            raise Exception('unknown src operand type')

        if desttype == 'r':
            newval = self.registers[dest] - val
            self.registers[dest] = newval
        elif desttype == 'm':
            addr = self._memory_operand(dest)
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
            addr = self._memory_operand(src)
            val = self.memory[addr] & 0xffff
        elif srctype == 'i':
            val = src & 0xffff
        else:
            raise Exception('unknown src operand type')

        if desttype == 'r':
            self.registers[dest] += val
        elif desttype == 'm':
            addr = self._memory_operand(dest)
            self.memory[addr] += val
        else:
            raise Exception('unknown dest operand type')

    def op_mul(self, operands):
        dest, src = operands

        dest, desttype = dest
        src, srctype = src

        if srctype == 'm' and desttype == 'm':
            raise Exception('invalid source,dest pair (%s)' % (operands))
        if desttype != 'r':
            raise Exception('invalid dest operand')

        if srctype == 'r':
            val = self.registers[src]
        elif srctype == 'm':
            addr = self._memory_operand(src)
            val = self.memory[addr] & 0xffff
        elif srctype == 'i':
            val = src & 0xffff
        else:
            raise Exception('unknown src operand type')

        if desttype == 'r':
            self.registers[dest] = (val * self.registers[dest]) & 0xffff
        elif desttype == 'm':
            addr = self._memory_operand(dest)
            self.memory[addr] = (val * self.memory[addr]) & 0xffff
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
            addr = self._memory_operand(src)
            val = self.memory[addr] & 0xff
            val |= self.memory[addr+1] << 0x8
        elif srctype == 'i':
            val = src & 0xffff
        else:
            raise Exception('unknown src operand type %s' % srctype)

        if desttype == 'r':
            self.registers[dest] = val
        elif desttype == 'm':
            addr = self._memory_operand(dest)
            if addr & 0xf000 != 0x7000:
                raise Exception(
                        f'runtime error: invalid memory address {addr:#06x}')
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
        elif op == 'mul':
            self.op_mul(operands)
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
