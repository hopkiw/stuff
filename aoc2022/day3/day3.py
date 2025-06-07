#!/usr/bin/env python3
"""day3"""


def getpriority(val):
    """getpriority"""
    ret = ord(val)
    return ret - 96 if ret > 90 else ret - 38


def main():
    """main"""
    lines = [
        'vJrwpWtwJgWrhcsFMMfFFhFp',
        'jqHRNqRjqzjGDLGLrsFMfFZSrLrFZsSL',
        'PmmdzqPrVvPwwTWBwg',
        'wMqvLMZHhHMvwLHjbvcjnnSBnvTQFn',
        'ttgJtRGJQctTZtZT',
        'CrZsJsPPZsGzwwsLwLmpwMDw',
        '',
    ]
    with open('input.txt', encoding='utf-8') as filehandle:
        lines = filehandle.read().splitlines()
    lines.append('')
    total = 0
    count = 0
    elves = []
    for line in lines:
        if count == 3:
            for letter in elves[0]:
                if letter in elves[1] and letter in elves[2]:
                    nnew = getpriority(letter)
                    print(letter, nnew)
                    total += nnew
                    count = 0
                    elves = []
                    break
        elves.append(line)
        count += 1
    print(total)


def mainpt1():
    """main"""
    lines = [
        'vJrwpWtwJgWrhcsFMMfFFhFp',
        'jqHRNqRjqzjGDLGLrsFMfFZSrLrFZsSL',
        'PmmdzqPrVvPwwTWBwg',
        'wMqvLMZHhHMvwLHjbvcjnnSBnvTQFn',
        'ttgJtRGJQctTZtZT',
        'CrZsJsPPZsGzwwsLwLmpwMDw',
    ]
    with open('input.txt', encoding='utf-8') as filehandle:
        lines = filehandle.read().splitlines()
    total = 0
    for line in lines:
        llen = len(line) // 2
        left, right = line[:llen], line[llen:]
        for letter in left:
            if letter in right:
                nnew = getpriority(letter)
                print(letter, nnew)
                total += nnew
                break
    print(total)


if __name__ == '__main__':
    main()
