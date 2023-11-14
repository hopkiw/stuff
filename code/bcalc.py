#!/usr/bin/env python3
"""Visualize binary bitwise operations"""

import rlcompleter
import readline
import sys

# TODO
# quit command
# sar, sign extension, 2s complement
# unary operators
# auto-detecting when to interpret result as 2s complement
# printing when twos complement has happened

BITS = 16


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
    elif op in ('shl', '<<'):
        res = (op1 << op2) & 0xffff
    elif op in ('shr', '>>'):
        res = op1 >> op2
    elif op in ('mul', '*'):
        res = op1 * op2
        if res & 0xffff != res:
            print('\noverflow!')
        res = res & 0xffff
    elif op in ('div', '/'):
        res = (int(op1 / op2), op1 % op2)
    elif op in ('add', '+'):
        res = op1 + op2
        if res & 0xffff != res:
            print('\noverflow!')
        res = res & 0xffff
    elif op in ('sub', '-'):
        res = (op1 - op2) & 0xffff
    elif op in ('not', '!'):
        res = 0xffff - op1
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
        if val & 0x8000 == 0x8000:
            footer = f'(2\'s complement: {-1 * (0xffff - val + 1)})'
        print(f'{op:<3}', _blue(f'{val:#018b}'), _green(f'{val:#06x}'), val,
              footer)

    print('\n')

    _printop(op1)
    if op2:
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
              '<<', 'shl',
              '>>', 'shr',
              '^', 'xor')


def _parse(text):
    op2 = None
    ops = text.split()
    try:
        op, op1 = ops
        if op not in UNARY_OPS:
            raise Exception('unknown operation')
    except ValueError:
        op1, op, op2 = ops
        if op not in BINARY_OPS:
            raise Exception('unknown operation')

    op1 = int(op1, 0) & 0xffff
    if op2 is not None:
        op2 = int(op2, 0) & 0xffff

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
