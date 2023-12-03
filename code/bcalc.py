#!/usr/bin/env python3
"""Visualize binary bitwise operations"""

import rlcompleter  # noqa: F401
import readline
import sys

BITS = 16
MASK = (1 << BITS) - 1
SIGNBIT = 1 << (BITS - 1)


class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'


def _exec(op, op1, op2):
    # TODO: add sar, imul, idiv
    if op in ('and', '&'):
        res = op1 & op2
    elif op in ('or', '|'):
        res = op1 | op2
    elif op in ('xor', '^'):
        res = op1 ^ op2
    elif op in ('sal', 'shl', '<<'):
        res = (op1 << op2) & MASK
    elif op in ('shr', '>>'):
        res = op1 >> op2
    elif op == 'sar':
        res = op1 >> op2
        if op1 & SIGNBIT == SIGNBIT:
            mask = ((1 << op2) - 1) << (BITS - op2)
            res = res | mask
    elif op in ('mul', '*'):
        res = op1 * op2
        res2 = (res & 0xffff0000) >> 16
        res = res & MASK
        res = (res, res2)
    elif op in ('div', '/'):
        res = (int(op1 / op2), op1 % op2)
    elif op in ('add', '+'):
        res = op1 + op2
        if res & MASK != res:
            print('result overflow')
        res = res & MASK
    elif op in ('sub', '-'):
        res = (op1 - op2) & MASK
    elif op in ('not', '!'):
        res = MASK - op1
    elif op == 'print':
        res = op1
    else:
        raise Exception('Unsupported operator "%s"' % op)

    return (res,) if isinstance(res, int) else res


def _blue(val):
    return f'{bcolors.OKBLUE}{val}{bcolors.ENDC}'


def _green(val):
    return f'{bcolors.OKGREEN}{val}{bcolors.ENDC}'


def _red(val):
    return f'{bcolors.FAIL}{val}{bcolors.ENDC}'


def _print(op, op1, op2, res, res2=None):
    def _printop(val, op=''):
        footer = ''
        if val & SIGNBIT == SIGNBIT:
            footer = f'(2c: {-1 * (MASK - val + 1)})'
        print(f'{op:<3}', _blue(f'{val:#018b}'), _green(f'{val:#06x}'), val,
              footer)

    print('\n')

    if op == 'print':
        _printop(op1)
        print('\n')
        return

    _printop(op1)
    if op2 is not None:
        _printop(op2, op)

    print('    -------------------------')

    _printop(res, '=')

    if res2:
        _printop(res2, '=')

    print('\n')


UNARY_OPS = ('not', 'neg')
BINARY_OPS = ('+', 'add',
              '-', 'sub',
              '/', 'div',
              '*', 'mul',
              '%', 'mod',
              '&', 'and',
              '|', 'or',
              '<<', 'shl', 'sal',
              '>>', 'shr',
              'sar',
              '^', 'xor')


def _parse(text):
    op2 = None
    ops = text.split()
    if len(ops) == 1:
        op = 'print'
        op1 = ops[0]
    elif len(ops) == 2:
        op, op1 = ops
        if op not in UNARY_OPS:
            raise Exception('unknown operation')
    elif len(ops) == 3:
        op1, op, op2 = ops
        if op not in BINARY_OPS:
            raise Exception('unknown operation')

    op1 = int(op1, 0)
    if op1 & MASK != op1:
        print('operand 1 overflow')
    op1 = op1 & MASK
    if op2 is not None:
        op2 = int(op2, 0)
        if op2 & MASK != op2:
            print('operand 2 overflow')
        op2 = op2 & MASK

    return op, op1, op2


def _input():
    while True:
        res = input('$ ')
        if not res:
            continue

        try:
            return _parse(res)
        except ValueError:
            print('Not enough arguments')


def main():
    loop = False
    if len(sys.argv) == 4:
        op1, op, op2 = sys.argv[1:]
    elif len(sys.argv) == 1:
        loop = True
        print('Welcome')
        try:
            readline.read_history_file()
        except FileNotFoundError:
            pass  # who care
        op, op1, op2 = _input()
    else:
        print('Usage:', sys.argv[0], '[<operand> <operator> <operand>]')
        return

    while True:
        try:
            res = _exec(op, op1, op2)
            _print(op, op1, op2, *res)
        except Exception as e:
            print(_red('Exception:'), e)
            # raise

        if not loop:
            break

        try:
            op, op1, op2 = _input()
        except (KeyboardInterrupt, EOFError):
            break

    readline.write_history_file()


if __name__ == '__main__':
    try:
        main()
    except Exception as e:
        print('Exited with exception:', e)
        raise

    print('Done')
