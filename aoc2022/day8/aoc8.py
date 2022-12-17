#!/usr/bin/env python3


# Walk the map;
# If first row or last row or first col or last col:
#   count += 1
#   continue
# for check in [up, right, down, left]:
#   iterate the map in direction:
#     if edge:
#       count +=1
#       break 2  # stop checking this tile
#     if check > original:
#       break 1  # stop checking this direction
# print(count)

import sys


def read_input_file(filename):
  lines = []
  with open(filename) as thefile:
    for line in thefile:
      lines.append(line.rstrip())

  return lines


def parse_map(lines):
  forest_map = []
  for line in lines:
    thisline = []
    for col in line:
      thisline.append(int(col))
    forest_map.append(thisline)

  return forest_map


def check_left(row, col, forest_map):
  # from left, checking each col of the row
  tile = forest_map[row][col]
  count = 0
  for i in range(col - 1, -1, -1):
    count += 1
    if forest_map[row][i] >= tile:
      # not visible
      return False, count

  return True, count


def check_top(row, col, forest_map):
  # from top, checking this col of each row
  tile = forest_map[row][col]
  count = 0
  for i in range(row - 1, -1, -1):
    count += 1
    if forest_map[i][col] >= tile:
      return False, count

  return True, count


def check_bottom(row, col, forest_map):
  # from tile to bottom, checking this col of each row
  tile = forest_map[row][col]
  count = 0
  for i in range(row + 1, len(forest_map)):
    count += 1
    if forest_map[i][col] >= tile:
      print(row, col, 'is blocked from the bottom by', i, col)
      return False, count

  return True, count


def check_right(row, col, forest_map):
  # from tile to right, checking each col of the row
  tile = forest_map[row][col]
  count = 0
  for i in range(col + 1, len(forest_map[0])):
    count += 1
    if forest_map[row][i] >= tile:
      # not visible
      print(row, col, 'is blocked to the right by', row, i)
      return False, count

  return True, count


def check_visible(row, col, forest_map):

  # Edge checks

  if row == 0 or col == 0:
    return True

  if row + 1 == len(forest_map):
    return True

  if col + 1 == len(forest_map[0]):
    return True

  # Direction checks

  if check_left(row, col, forest_map)[0]:
    return True

  if check_right(row, col, forest_map)[0]:
    return True

  if check_top(row, col, forest_map)[0]:
    return True

  if check_bottom(row, col, forest_map)[0]:
    return True

  return False


def scenic_score(row, col, forest_map):
  return check_left(row, col, forest_map)[1] * check_right(row, col, forest_map)[1] * check_top(row, col, forest_map)[1] * check_bottom(row, col, forest_map)[1]


def main():
  lines = read_input_file(sys.argv[1])
  print('Got', len(lines), 'lines')

  forest_map = parse_map(lines)
  for row in forest_map:
    print(row)

  count = 0
  rows = len(forest_map)
  cols = len(forest_map[0])
  for row in range(rows):
    for col in range(cols):
      if check_visible(row, col, forest_map):
        count += 1

  print('count', count)
  # assert count == 1703

  max_score = 0
  for row in range(rows):
    for col in range(cols):
      score = scenic_score(row, col, forest_map)
      if score > max_score:
        max_score = score

  print('score', max_score)

if __name__ == '__main__':
  main()
