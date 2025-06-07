#!/usr/bin/env python3
"""day1"""

from enum import Enum

class Sign(Enum):
    ROCK = 1
    PAPER = 2
    SCISSORS = 3

class Goal(Enum):
    LOSE = 1
    DRAW = 2
    WIN = 3


ROCKU =     'X'
PAPERU =    'Y'
SCISSORSU = 'Z'

ROCKT =     'A'
PAPERT =    'B'
SCISSORST = 'C'

LOSE = 'X'
DRAW = 'Y'
WIN =  'Z'


def tosign(val):
    """tosign"""
    if val in ('A', 'X'):
        return Sign.ROCK
    if val in ('B', 'Y'):
        return Sign.PAPER
    if val in ('C', 'Z'):
        return Sign.SCISSORS
    raise ValueError('Unknown sign')


def togoal(val):
    """togoal"""
    if val == 'X':
        return Goal.LOSE
    if val == 'Y':
        return Goal.DRAW
    if val == 'Z':
        return Goal.WIN
    raise ValueError('Unknown goal')

WINNING = {
    Sign.ROCK: Sign.PAPER,
    Sign.PAPER: Sign.SCISSORS,
    Sign.SCISSORS: Sign.ROCK
}

LOSING = {
    Sign.ROCK: Sign.SCISSORS,
    Sign.PAPER: Sign.ROCK,
    Sign.SCISSORS: Sign.PAPER
}


def main():
    """main"""
    with open('input.txt', encoding='utf-8') as filehandle:
        lines = filehandle.read().splitlines()
    # lines = ['A Y', 'B X', 'C Z']
    score = 0
    for line in lines:
        them, goal = line.split()
        them = tosign(them)
        goal = togoal(goal)
        move = None
        if goal == Goal.DRAW:
            score += 3
            move = them
        elif goal == Goal.WIN:
            score += 6
            move = WINNING[them]
        elif goal == Goal.LOSE:
            move = LOSING[them]
        score += int(move.value)
    print(score)


def mainpt2():
    """main"""
    with open('input.txt') as fh:
        lines = fh.read().splitlines()
    # lines = ['A Y', 'B X', 'C Z']
    score = 0
    for line in lines:
        them, goal = line.split()
        us = None
        if goal == DRAW:
            score += 3
            if them == ROCKT:
                us = ROCKT 
            if them == PAPERT:
                us = PAPERT
            if them == SCISSORST:
                us = SCISSORST
        elif goal == WIN:
            score += 6
            if them == ROCKT:
                us = PAPERT
            if them == PAPERT:
                us = SCISSORST
            if them == SCISSORST:
                us = ROCKT
        elif goal == LOSE:
            if them == ROCKT:
                us = SCISSORST
            if them == PAPERT:
                us = ROCKT
            if them == SCISSORST:
                us = PAPERT
        if us == ROCKT:
            score += 1
        elif us == PAPERT:
            score += 2
        elif us == SCISSORST:
            score += 3
    print(score)




def mainpt1():
    """main"""
    with open('input.txt') as fh:
        lines = fh.read().splitlines()
    # lines = ['A Y', 'B X', 'C Z']
    score = 0
    for line in lines:
        them, us = line.split()
        if us == ROCKU:
            score += 1
            if them == ROCKT:
                print('draw')
                score += 3
            if them == SCISSORST:
                print('rock beats scissors')
                score += 6
        elif us == PAPERU:
            score += 2
            if them == PAPERT:
                print('draw')
                score += 3
            if them == ROCKT:
                print('paper beats rock')
                score += 6
        elif us == SCISSORSU:
            score += 3
            if them == SCISSORST:
                print('draw')
                score += 3
            if them == PAPERT:
                print('scissors beats paper')
                score += 6
        print(score)
    print(score)



if __name__ == '__main__':
    main()
