#!/usr/bin/python

import random
import time

options = ["rock", "paper", "scissors"]
seeded = False


def generate_play_string(choices):
    string = ""
    for num in range(len(choices)):
        string += choices[num].capitalize()
        if num < len(choices) - 1:
            string += ", "
    return string + ": "


def get_computer_choice():
    global seeded
    if not seeded:
        random.seed(time.time())
        seeded = True
    return random.randint(0, 2)


def play_again():
    choice = input("Play again? Y/n ").lower()
    return (choice == "y" or choice == "")


def main():
    play_string = generate_play_string(["Rock", "Paper", "Scissors"])
    while True:
        player = input(play_string).lower()
        if player not in options:
            print("Hey, that's not one of the options!")
            continue
        player = options.index(player)
        computer = get_computer_choice()

        if player == computer:
            print("It's a draw! We both picked %s" % options[player])
        elif computer == player + 1 or (computer == 0 and player == len(options) - 1):
            print("Computer wins! %s beats %s" %
                  (options[computer], options[player]))
        else:
            print("Player wins! %s beats %s" %
                  (options[player], options[computer]))

        if not play_again():
            break


if __name__ == "__main__":
    main()
