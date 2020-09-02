#!/usr/bin/python

from rock_paper_scissors import generate_play_string


def test_generate_play_string():
    res = generate_play_string(["rock", "paper", "scissors"])
    assert res == "Rock, Paper, Scissors: "

    res = generate_play_string(["hello", "world"])
    assert res == "Hello, World: "
