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
  def __init__(self, name, email, address):
    self.name = name
    self.email = email
    # Future fields:
    # Prefix, title
    # First name
    # Middle name
    # Last, family name
    # Workplace
    # Phone number, with type
    # Email address
    # Address
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


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('action', choices=['add', 'delete', 'search'])
  parser.add_argument('--name')
  parser.add_argument('--email')
  parser.add_argument('--address')
  args = parser.parse_args()

  # filter: Return a sequence yielding those items of iterable for which
  # function(item) is true. If function is None, return the items that are
  # true.
  if not filter(None, [args.name, args.email, args.address]):
    parser.error('Must supply at least one attribute.')

  # Pre-seed the entries list with a fake entry.
  entries.append(Entry(name="Fake name", email="fake@email.gov",
                       address="123 fake street"))

  if args.action == 'add':
    entry_args = args.__dict__
    entry_args.pop('action')
    entries.append(Entry(**args.__dict__))
  elif args.action == 'delete':
    for idx in range(len(entries)):
      entry = entries[idx]
      # TODO: If user specifies two fields, only delete those that match both.
      if (entry.name == args.name or entry.email == args.email or
          entry.address == args.address):
        entries.pop(idx)
  elif args.action == 'search':
    for entry in entries:
      if (entry.name == args.name or entry.email == args.email or
          entry.address == args.address):
        print(entry)


if __name__ == '__main__':
  main()
