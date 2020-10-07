#!/usr/bin/python
import sys
import argparse

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

entries = []


class Entry(object):
  def __init__(self, name, email='', address='', title='', first_name='', middle_name='', last_name='', workplace='', phone=''):
    self.name = name
    self.email = email
    self.address = address
    self.title = title
    self.first_name = first_name
    self.middle_name = middle_name
    self.last_name = last_name
    self.workplace = workplace
    self.phone = phone

    # Future fields:
    # Instant messenger handle, with service
    # Important dates/Anniversaries
    # Notes
    # Relationship
    # Website
    # Label (though this may not be implemented in an entry)
    # Photograph
    # GPG key
    # Gender
    # Contact preferences
    # Reference to external calendar
    # VCard

  def __str__(self):
    return 'Entry({}, {})'.format(self.name, self.email)

  def __repr__(self):
    return self.__str__()

  def info(self):
    if self.first_name and self.last_name:
      if self.title:
        print(self.title + ' ' + self.first_name + ' ' + self.last_name)
      else:
        print(self.first_name + ' ' + self.last_name)
    else:
      print(self.name)
    print('Email: ' + self.email)
    print('Phone: ' + self.phone)

# If csv exist, load into memory in set of entry objects and returns a list
# Load before any action
# When you add or delete, if this is first entry, create file. Otherwise write
# during add/delete.
thefile = "./fake.csv"

def load_from_csv():
  with open(thefile) as f:
    lines = f.read().splitlines()
  entries = []
  for line in lines:
    fields = line.split(",")
    entries.append(Entry(*fields))

  return entries

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

  print("args: {}".format(args))

  # filter: Return a sequence yielding those items of iterable for which
  # function(item) is true. If function is None, return the items that are
  # true.
  if not filter(None, [args.name, args.email, args.address]):
    parser.error('Must supply at least one attribute.')

  # Pre-seed the entries list with a fake entry.
  # TODO: load csv
  #
  entries = load_from_csv()
  # entries.append(Entry(name="Fake name", title='Mr.', first_name='Fakest',
  #                     last_name='Name', email="fake@email.gov",
  #                     address="123 fake street", phone='1-800-FAKE-NUMB'))

  if args.action == 'add':
    entry_args = args.__dict__
    entry_args.pop('action')
    e = Entry(**entry_args)
    entries.append(e)
  elif args.action == 'delete':
    for idx in range(len(entries)):
      entry = entries[idx]
      if args.name and entry.name != args.name:
        continue
      elif args.email and entry.email != args.email:
        continue
      elif args.address and entry.address != args.address:
        continue
      elif args.title and entry.title != args.title:
        continue
      elif args.first_name and entry.first_name != args.first_name:
        continue
      elif args.middle_name and entry.middle_name != args.middle_name:
        continue
      elif args.last_name and entry.last_name != args.last_name:
        continue
      elif args.workplace and entry.workplace != args.workplace:
        continue
      elif args.phone and entry.phone != args.phone:
        continue
      entries.pop(idx)
  elif args.action == 'search':
    for entry in entries:
      if args.name and entry.name == args.name:
        entry.info()
      elif args.email and entry.email == args.email:
        entry.info()
      elif args.address and entry.address == args.address:
        entry.info()
      elif args.title and entry.title == args.title:
        entry.info()
      elif args.first_name and entry.first_name == args.first_name:
        entry.info()
      elif args.middle_name and entry.middle_name == args.middle_name:
        entry.info()
      elif args.last_name and entry.last_name == args.last_name:
        entry.info()
      elif args.workplace and entry.workplace == args.workplace:
        entry.info()
      elif args.phone and entry.phone == args.phone:
        entry.info()


if __name__ == '__main__':
  main()
