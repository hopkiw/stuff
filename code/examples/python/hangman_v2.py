#!/usr/bin/python

import random
import time

# 'Constants'
MAX_GUESSES = 6
MIN_WORD_LENGTH = 3


class Hangman(object):
  def __init__(self):
    # Initialize instance variables
    self.guessed = set()
    self.seeded = False
    self.the_word = None
    self.user_visible = None

  def pick_word_from_dictionary(self):
    if self.the_word:
      return

    if not self.seeded:
      random.seed(time.time())
      self.seeded = True

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

    self.the_word = random.choice(guessable_words)

  def generate_display(self):
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

    wordline = "|     " + self.user_visible

    ret = [
        cap,
        "|",
    ]
    ret += top
    ret += body_frames[len(self.guessed)]
    ret += bottom
    ret += [
        "|",
        wordline,
        "|",
        cap,
    ]
    width = 25

    # pythonic one-liner:
    # return "\n".join([x + (" " * (width - len(x))) + "|" for x in ret])

    # explicit method:
    real_ret = ""
    for line in ret:
      real_ret += line
      real_ret += " " * (width - len(line))
      real_ret += "|\n"

    return real_ret

  def update_user_visible_word(self):
      ret = ""
      for letter in self.the_word:
          if letter in self.guessed:
              ret += letter
          else:
              ret += "_"

      self.user_visible = ret

  def play(self):
    print("Starting game")
    self.pick_word_from_dictionary()

    while True:
      # "Draw the game"
      self.update_user_visible_word()
      print(self.generate_display())

      # "Validate statuses"
      if self.user_visible == self.the_word:
        print("You win!")
        break
      elif len(self.guessed) >= MAX_GUESSES:
        print("Out of guesses, sorry! The word was {}".format(self.the_word))
        break
      else:
        print("{} guesses left\n".format(MAX_GUESSES - len(self.guessed)))

      # Guess a letter
      guess = raw_input("Guess a letter: ").lower()
      if not guess.isalpha() or len(guess) != 1:
        print("just a letter, please.")
        continue
      if guess in self.guessed:
        print("you already guessed %s" % guess)
        continue
      self.guessed.add(guess)


def main():
  game = Hangman()
  game.play()


if __name__ == "__main__":
    main()
