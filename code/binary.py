#!/usr/bin/env python3
"""Visualize inary bitwise operations"""

import sys


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
    if op in ('and', '&'):
        res = op1 & op2
    elif op in ('or', '|'):
        res = op1 | op2
    elif op in ('xor', '^'):
        res = op1 ^ op2
    elif op in ('shl', '<<'):
        res = op1 << op2
    elif op in ('shr', '>>'):
        res = op1 >> op2
    elif op in ('mul', '*'):
        res = op1 * op2
    elif op in ('div', '/'):
        res = int(op1 / op2)
    elif op in ('add', '+'):
        res = op1 + op2
    else:
        raise Exception('Unsupported operator "%s"' % op)

    return res


def _print(op, op1, op2, res):
    line = (f'    {bcolors.OKBLUE}{op1:#016b}{bcolors.ENDC} '
            f'{bcolors.OKGREEN}{op1:#06x}{bcolors.ENDC} ({op1})')
    print('\n')
    print(line)
    line = (f'{op:<3} {bcolors.OKBLUE}{op2:#016b}{bcolors.ENDC} '
            f'{bcolors.OKGREEN}{op2:#06x}{bcolors.ENDC} ({op2})')
    print(line)
    print('     -------------------------')
    line = (f'  = {bcolors.OKBLUE}{res:#016b}{bcolors.ENDC} '
            f'{bcolors.OKGREEN}{res:#06x}{bcolors.ENDC} ({res})')
    print(line, '\n')


def _input():
    while True:
        res = input('$ ')
        if not res:
            continue
        try:
            op1, op, op2 = res.split()
        except Exception:
            print('Error')
            continue

        op1 = int(op1, 0) & 0xffff
        op2 = int(op2, 0) & 0xffff

        return op, op1, op2


def main():
    if len(sys.argv) == 4:
        op1, op, op2 = sys.argv[1:]
    elif len(sys.argv) == 1:
        loop = True
        print('Welcome')
        op, op1, op2 = _input()
    else:
        print('Usage:', sys.argv[0], '[<operand> <operator> <operand>]')
        return

    while True:
        res = _exec(op, op1, op2)
        _print(op, op1, op2, res)

        if not loop:
            break

        op, op1, op2 = _input()


if __name__ == '__main__':
    try:
        main()
    except (KeyboardInterrupt, EOFError):
        print('Done')
