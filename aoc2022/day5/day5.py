#!/usr/bin/env python3
"""day5"""


def parse_move(desc):
    """parse_move"""
    print('parsing ', desc)
    desc = desc.split()
    count = int(desc[1])
    src = int(desc[3]) - 1
    dst = int(desc[5]) - 1
    return [count, src, dst]


def make_move(table, move):
    """make_move"""
    imove = parse_move(move)
    print('got imove:', imove)
    count, src, dst = imove
    print("move ", count, " from ", src, " to ", dst)
    tomove = table[src][0 - count:]
    table[src] = table[src][:0 - count]
    table[dst].extend(tomove)


def main():
    """main"""
    lines = [
            '    [D]    ',
            '[N] [C]    ',
            '[Z] [M] [P]',
            ' 1   2   3',
            '',
             'move 1 from 2 to 1',
             'move 3 from 1 to 3',
             'move 2 from 2 to 1',
             'move 1 from 1 to 2',
    ]
    with open('input.txt') as fh:
        lines = fh.read().splitlines()
    for i, line in enumerate(lines):
        if not line:
            break
    table_desc = lines[:i]
    moves = lines[i+1:]
    table = parse_table(table_desc)
    print('got table: ', table)
    for move in moves:
        make_move(table, move)
        print('table is now', table)
    print(''.join([stack[-1] for stack in table]))


def parse_table(desc):
    """parse_table"""
    cols = int(desc[-1].split()[-1])
    print('cols', cols)
    table = [[] for i in range(cols)]
    for line in reversed(desc[:-1]):
        x = 1
        for i in range(cols):
            val = line[x]
            if val != ' ':
                table[i].append(val)
            x += 4
            
    # return [['Z', 'N'], ['M', 'C', 'D'], ['P']]
    return table


def parse_movept1(desc):
    """parse_move"""
    print('parsing ', desc)
    desc = desc.split()
    count = int(desc[1])
    src = int(desc[3]) - 1
    dst = int(desc[5]) - 1
    return [[src, dst] for i in range(1, count+1)]


def make_movept1(table, move):
    """make_move"""
    for imove in parse_move(move):
        print('got imove:', imove)
        src, dst = imove
        print("move 1 from ", src, " to ", dst)
        table[dst].append(table[src].pop())


def mainpt1():
    """main"""
    lines = [
            '    [D]    ',
            '[N] [C]    ',
            '[Z] [M] [P]',
            ' 1   2   3',
            '',
             'move 1 from 2 to 1',
             'move 3 from 1 to 3',
             'move 2 from 2 to 1',
             'move 1 from 1 to 2',
    ]
    with open('input.txt') as fh:
        lines = fh.read().splitlines()
    for i, line in enumerate(lines):
        if not line:
            break
    table_desc = lines[:i]
    moves = lines[i+1:]
    table = parse_table(table_desc)
    print('got table: ', table)
    for move in moves:
        make_movept1(table, move)
        print('table is now', table)
    print(''.join([stack[-1] for stack in table]))


if __name__ == '__main__':
    main()
