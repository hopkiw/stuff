#!/usr/bin/env python3
"""day1"""


def main():
    """main"""
    with open('input.txt') as fh:
        lines = fh.read().splitlines()
    elves = []
    elf = 0
    for line in lines:
        if not line:
            elves.append(elf)
            elf = 0
            continue
        elf += int(line)
    elves.sort()
    print(sum(elves[-3:]))
    for i in elves[-3:]:
        print(i)


def mainpt1():
    """main"""
    with open('input.txt') as fh:
        lines = fh.read().splitlines()
    biggest = 0
    elf = 0
    for line in lines:
        if not line:
            if elf > biggest:
                biggest = elf
            elf = 0
            continue
        elf += int(line)
    print(biggest)
def mainpt1():
    """main"""
    with open('input.txt') as fh:
        lines = fh.read().splitlines()
    biggest = 0
    elf = 0
    for line in lines:
        if not line:
            if elf > biggest:
                biggest = elf
            elf = 0
            continue
        elf += int(line)
    print(biggest)


if __name__ == '__main__':
    main()
