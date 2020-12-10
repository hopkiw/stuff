#!/usr/bin/python

import argparse
from address_book import address_book

# An addressbook entry can have many fields, some of which may be unique and
# some which may be repeated. The design for addressbook entries should not
# aim to fully define all possible fields at once, but permit extensibility.
#
# The entire addressbook should be viewable, with optional filtering. A
# comprehensive interface (CLI, TUI, GUI, or Web) should permit viewing,
# filtering (searching), adding, deleting and modifying entries. A special case
# of viewing is exporting, which should be supported in any interface.
# Similarly, a special case of adding an entry is importing, whether one entry
# or bulk.
#
# In order to support multiple interfaces, the addressbook format and DB
# manipulation routines should be stored in a package.

fakefile = "./fake.csv"



# TODO: add search_for_deletion to package and leave prompt_for_deletion in program




def prompt_for_deletion(book,args):
    idx = book.search_for_deletion(args)
    print(book.entries[idx])
    inp = raw_input('Is this the entry you wish to delete? (Y/N) ').upper()
    if inp == 'Y':
      book.entries.pop(idx)
      return True
    return False



def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('action', choices=['add', 'delete', 'search'])
  parser.add_argument('--name')
  parser.add_argument('--email')
  parser.add_argument('--address')
  parser.add_argument('--title')
  parser.add_argument('--first_name')
  parser.add_argument('--middle_name')
  parser.add_argument('--last_name')
  parser.add_argument('--workplace')
  parser.add_argument('--phone')
  args = parser.parse_args()

  if not filter(None, [args.name, args.email, args.address]):
    parser.error('Must supply at least one attribute.')

  book = address_book.Book()
  book.load_from_csv(fakefile)


  if args.action == 'add':
    entry_args = args.__dict__
    entry_args.pop('action')
    e = address_book.Entry(**entry_args)
    book.entries.append(e)
    book.save_to_csv(fakefile)
  elif args.action == 'delete':
    if prompt_for_deletion():
      book.save_to_csv(fakefile)
  elif args.action == 'search':
    matches = book.get_matching_entries(args)
    for match in matches:
      print(match)


if __name__ == '__main__':
  main()
