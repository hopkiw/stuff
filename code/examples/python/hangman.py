#!/usr/bin/python

import random
import time

# 'Constants'
MAX_GUESSES = 6
MIN_WORD_LENGTH = 3

# Global variables
guessed = set()
seeded = False


def pick_word_from_dictionary():
  global seeded
  if not seeded:
    random.seed(time.time())
    seeded = True

  with open("/usr/share/dict/words") as word_list_file:
    # pythonic one-liner:
    #
    # guessable_words = \
    #   [x for x in word_list_file.read().splitlines() if x.isalpha() and
    #       x.islower() and len(x) > MIN_WORD_LENGTH]

    # explicit way:
    all_words = word_list_file.read().splitlines()
    guessable_words = []
    for word in all_words:
      if word.isalpha() and word.islower() and len(word) > MIN_WORD_LENGTH:
        guessable_words.append(word)

  return random.choice(guessable_words)


def generate_display(word):
  cap = "+-----------------------+"
  top = [
      "|   +------+  ",
      "|   |      |  ",
  ]
  bottom = [
      "|   |         ",
      "|   |         ",
      "|  _|_        ",
  ]

  body_frames = [
      [
          "|   |         ",
          "|   |         ",
          "|   |         ",
          "|   |         ",
      ],
      [
          "|   |      o  ",
          "|   |         ",
          "|   |         ",
          "|   |         ",
      ],
      [
          "|   |      o  ",
          "|   |      |  ",
          "|   |         ",
          "|   |         ",
      ],
      [
          "|   |      o  ",
          "|   |     /|  ",
          "|   |         ",
          "|   |         ",
      ],
      [
          "|   |      o  ",
          "|   |     /|\\ ",
          "|   |         ",
          "|   |         ",
      ],
      [
          "|   |      o  ",
          "|   |     /|\\ ",
          "|   |      |  ",
          "|   |     /   ",
      ],
      [
          "|   |      o  ",
          "|   |     /|\\ ",
          "|   |      |  ",
          "|   |     / \\ ",
      ],
  ]

  wordline = "|     " + word

  ret = [
      cap,
      "|",
  ]
  ret += top
  ret += body_frames[len(guessed)]
  ret += bottom
  ret += [
      "|",
      wordline,
      "|",
      cap,
  ]
  width = 25

  return "\n".join([x + (" " * (width - len(x))) + "|" for x in ret])


def generate_word(the_word):
    global guessed
    ret = ""
    for letter in the_word:
        if letter in guessed:
            ret += letter
        else:
            ret += "_"

    return ret


def play():
  the_word = pick_word_from_dictionary()
  while True:
    # "Draw the game"
    user_visible = generate_word(the_word)
    print(generate_display(user_visible))

    # "Validate statuses"
    if user_visible == the_word:
      print("You win!")
      break
    elif len(guessed) >= MAX_GUESSES:
      print("Out of guesses, sorry! The word was {}".format(the_word))
      break
    else:
      print("{} guesses left\n".format(MAX_GUESSES - len(guessed)))

    # Guess a letter
    guess = raw_input("Guess a letter: ").lower()
    if not guess.isalpha() or len(guess) != 1:
      print("just a letter, please.")
      continue
    if guess in guessed:
      print("you already guessed %s" % guess)
      continue
    guessed.add(guess)


def main():
  print("Starting game")
  play()


if __name__ == "__main__":
    main()
