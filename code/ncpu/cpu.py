#!/usr/bin/env python3

from collections import defaultdict
from collections.abc import MutableMapping
from enum import Enum


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


class OpType(Enum):
    MEMORY = 'm'
    IMMEDIATE = 'i'
    REGISTER = 'r'
    DATAL = 'dl'
    TEXTL = 'tl'


class Operand:
    def __init__(self, optype, value):
        if optype not in OpType:
            raise Exception('invalid optype %s' % optype)

        self.optype = optype
        self.value = value


class Memory(MutableMapping):
    def __init__(self, memory=None):
        self.memory = defaultdict(int)
        if memory:
            self.update(memory)

    def copy(self):
        return Memory(self.memory)

    def __getitem__(self, key):
        return self.memory.__getitem__(key)

    def __delitem__(self, key):
        return self.memory.__delitem__(key)

    def __setitem__(self, key, value):
        return self.memory.__setitem__(key, value)

    def __iter__(self):
        return self.memory.__iter__()

    def __len__(self):
        return self.memory.__len__()


class State:
    def __init__(self, registers=None, flags=None, memory=None):
        self.registers = {reg: 0 for reg in CPU._registers}
        if registers:
            self.registers.update(registers)

        self.flags = {'zf': 0, 'cf': 0, 'of': 0}
        if flags:
            self.flags.update(flags)

        if memory:
            self.memory = memory
        else:
            self.memory = Memory()

    def __repr__(self):
        return f'State({self.registers=}, {self.flags=}, {self.memory=})'

    def copy(self):
        return State(self.registers, self.flags, self.memory)


class CPU:
    _registers = ['ax', 'bx', 'cx', 'dx', 'si', 'di', 'ip', 'bp', 'sp']

    def __init__(self, program, offset=0, data=None, state=None):
        if data and state:
            raise Exception('initial data and state cannot both be provided')

        self.instructions = self._rewrite_labels(program.copy())
        self.data = data  # to detect if data is present

        if state:
            self.states = [state.copy()]
        else:
            self.states = [State()]
            self.states[-1].registers['sp'] = STACK
            self.states[-1].registers['bp'] = 0x1
            self.states[-1].registers['ip'] = offset + TEXT
            if data:
                self._load(data)

    def _load(self, data):
        for addr in data:
            self.memory[addr + DATA] = data[addr] & 0xffff

    def _rewrite_labels(self, instructions):
        newi = []
        for i in range(len(instructions)):
            op, operands = instructions[i]
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
            newi.append((op, new_operands))

        return newi

    def prev(self):
        if len(self.states) > 1:
            self.states.pop()

    def _get_operand_value(self, operand, optype):
        if optype == 'r':
            return self.registers[operand]
        elif optype == 'm':
            addr = self._memory_operand(operand)
            return self._read_memory(addr)
        elif optype == 'i':
            return operand & 0xffff
        else:
            raise Exception('unknown operand type %s' % optype)

    def _set_operand_value(self, operand, optype, value):
        if optype == 'r':
            self.registers[operand] = value & 0xffff
        elif optype == 'm':
            addr = self._memory_operand(operand)
            self._write_memory(addr, value)
        else:
            raise Exception('unknown operand type %s' % optype)

    def _read_memory(self, addr):
        b1, b2 = self.memory[addr], self.memory[addr+1]
        return (b2 << 8) | b1

    def _write_memory(self, addr, value):
        value = value & 0xffff
        self.memory[addr] = value & 0xff    # lower byte
        self.memory[addr+1] = value >> 0x8  # upper byte

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

    def op_ret(self):
        # TODO: annoying use of asm which cpu should not know much about - will
        # replace with operand classes
        self.op_jmp(*parse_operands('[sp]'))
        self.op_add(*parse_operands('sp,0x02'))

    def op_call(self, operand):
        self.op_push(parse_operands('ip')[0])
        self.op_jmp(operand)

    def op_push(self, operand):
        self.op_sub(*parse_operands('sp,0x02'))
        stack = parse_operands('[sp]')
        self.op_mov(stack[0], operand, force=True)

    def op_pop(self, operand):
        stack = parse_operands('[sp]')[0]
        self.op_mov(operand, stack)
        self.op_add(*parse_operands('sp,0x02'))

    def op_jmp(self, operand):
        dest, desttype = operand
        addr = self._get_operand_value(dest, desttype)

        if addr & 0xff00 != TEXT:
            raise Exception('invalid jmp target %x' % addr)

        self.states[-1].registers['ip'] = addr

    def op_jne(self, operand):
        if self.flags['zf'] == 0:
            self.op_jmp(operand)

    def op_je(self, operand):
        if self.flags['zf'] == 1:
            self.op_jmp(operand)

    def op_cmp(self, dest, src):
        # dest, src = operands

        dest, desttype = dest
        src, srctype = src

        if srctype == 'm' and desttype == 'm':
            raise Exception('invalid source,dest pair (%s,%s)' % (dest, src))
        if desttype == 'i':
            raise Exception('invalid dest operand')

        srcval = self._get_operand_value(src, srctype)
        destval = self._get_operand_value(dest, desttype)

        if destval - srcval == 0:
            self.states[-1].flags['zf'] = 1
        else:
            self.states[-1].flags['zf'] = 0

    def op_sub(self, dest, src):
        # dest, src = operands

        dest, desttype = dest
        src, srctype = src

        if srctype == 'm' and desttype == 'm':
            raise Exception('invalid source,dest pair (%s,%s)' % (dest, src))
        if desttype == 'i':
            raise Exception('invalid dest operand')

        destval = self._get_operand_value(dest, desttype)
        srcval = self._get_operand_value(src, srctype)
        res = destval - srcval

        self._set_operand_value(dest, desttype, res)

        if res == 0:
            self.flags['zf'] = 1
        else:
            self.flags['zf'] = 0

    def op_add(self, dest, src):
        # dest, src = operands

        dest, desttype = dest
        src, srctype = src

        if srctype == 'm' and desttype == 'm':
            raise Exception('invalid source,dest pair (%s,%s)' % (dest, src))
        if desttype == 'i':
            raise Exception('invalid dest operand')

        opval = self._get_operand_value(src, srctype)
        val = self._get_operand_value(dest, desttype)
        self._set_operand_value(dest, desttype, val + opval)

    def op_mul(self, operand):
        op, optype = operand

        if optype not in ('r', 'm'):
            raise Exception('invalid operand "%s"', operand)

        opval = self._get_operand_value(op, optype)
        res = (opval * self.registers['ax']) & 0xffffffff
        self.registers['dx'] = res >> 16     # high byte
        self.registers['ax'] = res & 0xffff  # low byte

        if self.registers['dx'] == 0:
            self.flags['cf'] = 0
            self.flags['of'] = 0
        else:
            self.flags['cf'] = 1
            self.flags['of'] = 1

    def op_mov(self, dest, src, force=False):
        # dest, src = operands

        dest, desttype = dest
        src, srctype = src

        if srctype == 'm' and desttype == 'm' and not force:
            raise Exception('invalid source,dest pair (%s)'
                            % tuple([dest, src]))

        if desttype == 'i':
            raise Exception('invalid dest operand')

        if desttype == 'm':
            addr = self._memory_operand(dest)
            if addr & 0xf000 != 0x7000:
                raise Exception(
                        f'runtime error: invalid memory address {addr:#06x}')

        opval = self._get_operand_value(src, srctype)
        self._set_operand_value(dest, desttype, opval)

    def op_hlt(self):
        # TODO: handle this and simply display/stop taking input
        raise Exception('halt!')

    def op_nop(self):
        return

    def op_whatever(self, operands):
        # shl   r/m16,imm8
        # shl   r/m16,cl
        # shr   r/m16,imm8
        # shr   r/m16,cl
        # test  r/m16,imm16
        # xor   r/m16,imm16
        # xor   r/m16,r16
        # not   r/m16
        # or    r/m16,imm16
        # or    r/m16,r16
        # and   r/m16,imm16
        # and   r16,r/m16
        # out   imm8,ax
        # out   dx,ax
        # in    ax,imm8
        # in    ax,dx
        # mul   r/m16  # Unsigned multiply (DX:AX := AX ∗ r/m16).
        # div   r/m16  # Unsigned divide DX:AX by r/m16
        #              # with result stored in AX := Quotient, DX := Remainder.
        #
        # imul - after adding signed values, sign extension, etc.
        # idiv - ""
        # hlt
        # lea - after adding more addressing modes
        pass

    def execute(self):
        self.states.append(State(self.registers.copy(), self.flags.copy(),
                                 self.memory.copy()))

        ip = self.ip
        self.states[-1].registers['ip'] += 1
        op, operands = self.instructions[ip - TEXT]
        op = 'op_' + op
        opfunc = getattr(self, op, None)
        if opfunc:
            opfunc(*operands)
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
