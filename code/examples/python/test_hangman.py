#!/usr/bin/python

import hangman


def test_pick_word_sets_seeded():
  hangman.seeded = False
  hangman.pick_word_from_dictionary()
  # we assert using the 'is' operator, not ==, which would see if it evaluates
  # to True in a boolean context. That would allow "yes" or "a" or [1] to pass
  # the test.
  assert hangman.seeded is True


def test_pick_word_200_times():
  for f in range(200):
    res = hangman.pick_word_from_dictionary()
    assert len(res) > 0
    assert res.isalpha()


def test_generate_word_matched():
  test_word = "banana"

  hangman.guessed.clear()
  hangman.guessed.add("b")
  hangman.guessed.add("a")
  hangman.guessed.add("n")

  res = hangman.generate_word(test_word)
  assert res == test_word


def test_generate_word_partial():
  test_word = "banana"

  hangman.guessed.clear()
  hangman.guessed.add("b")
  hangman.guessed.add("a")

  res = hangman.generate_word(test_word)
  assert res == "ba_a_a"


def test_generate_display():
  hangman.guessed.clear()
  hangman.guessed.add("a")
  res = hangman.generate_display("test")
  assert res == """+-----------------------+|
|                        |
|   +------+             |
|   |      |             |
|   |      o             |
|   |                    |
|   |                    |
|   |                    |
|   |                    |
|   |                    |
|  _|_                   |
|                        |
|     test               |
|                        |
+-----------------------+|"""
