#!/bin/bash

[[ $1 == "" ]] && exit 1

winid=$(xwininfo -root -children | grep feh | awk '{print $1}')
/home/cc/bin/tagdb.py --get-all-tags | dmenu -l 10 -p 'Add tag: ' -fn inconsolata:size=22 -nb black -nf white -w $winid | while IFS=$'\n' read tag; do
  echo "trying to add $tag"
  /home/cc/bin/tagdb.py --add --tag "$tag" --path "$1"
done
