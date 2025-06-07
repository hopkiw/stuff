#!/usr/bin/env python3
"""day4"""



def main():
    """main"""
    lines = [
            '2-4,6-8',
            '2-3,4-5',
            '5-7,7-9',
            '2-8,3-7',
            '6-6,4-6',
            '2-6,4-8',
    ]
    with open('input.txt') as fh:
        lines = fh.read().splitlines()
    count = 0
    for line in lines:
        left, right = line.split(',')
        left = left.split('-')
        left = [int(x) for x in left]
        right = right.split('-')
        right = [int(x) for x in right]
        if left[0] <= right[0] and left[1] >= right[0]:
            count += 1
            continue
        if left[0] <= right[1] and left[1] >= right[1]:
            count += 1
            continue
        if right[0] <= left[0] and right[1] >= left[0]:
            count += 1
            continue
        if right[0] <= left[1] and right[1] >= left[1]:
            count += 1
            continue
    print(count)

def mainpt1():
    """main"""
    lines = [
            '2-4,6-8',
            '2-3,4-5',
            '5-7,7-9',
            '2-8,3-7',
            '6-6,4-6',
            '2-6,4-8',
    ]
    with open('input.txt') as fh:
        lines = fh.read().splitlines()
    count = 0
    for line in lines:
        left, right = line.split(',')
        left = left.split('-')
        left = [int(x) for x in left]
        right = right.split('-')
        right = [int(x) for x in right]
        if left[0] <= right[0] and left[1] >= right[1]:
            count += 1
            continue
        if right[0] <= left[0] and right[1] >= left[1]:
            count += 1
    print(count)


if __name__ == '__main__':
    main()
