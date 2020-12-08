
import csv
import os

entries = []

class Entry(object):
  def __init__(self, name, email='', address='', title='', first_name='',
               middle_name='', last_name='', workplace='', phone=''):
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
    if self.phone:
      return('Phone: ' + self.phone)
    elif self.email:
      return ('Email: ' + self.email)

  def __repr__(self):
    return self.__str__()

  def export(self):
    return [self.name, self.email, self.address, self.title, self.first_name,
            self.middle_name, self.last_name, self.workplace, self.phone]

# TODO: make test data and write test function for load_from_csv

def load_from_csv(thefile):
  entries = []
  if os.path.exists(thefile):
    with open(thefile) as f:
      lines = f.read().splitlines()
    for line in lines:
      fields = line.split(",")
      entries.append(Entry(*fields))
  return entries


def save_to_csv(entries, thefile):
  wr = csv.writer(open(thefile, "w+"), delimiter=',')
  for entry in entries:
    wr.writerow(entry.export())
def search_for_deletion(args):
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
    return idx

  def get_matching_entries(args):
      matches = []
      for entry in entries:
          if args.name and entry.name == args.name:
              matches.append(entry)
          elif args.email and entry.email == args.email:
              matches.append(entry)
          elif args.address and entry.address == args.address:
              matches.append(entry)
          elif args.title and entry.title == args.title:
              matches.append(entry)
          elif args.first_name and entry.first_name == args.first_name:
              matches.append(entry)
          elif args.middle_name and entry.middle_name == args.middle_name:
              matches.append(entry)
          elif args.last_name and entry.last_name == args.last_name:
              matches.append(entry)
          elif args.workplace and entry.workplace == args.workplace:
              matches.append(entry)
          elif args.phone and entry.phone == args.phone:
              matches.append(entry)
      return matches

def get_matching_entries(args):
  matches = []
  for entry in entries:
    if args.name and entry.name == args.name:
      matches.append(entry)
    elif args.email and entry.email == args.email:
      matches.append(entry)
    elif args.address and entry.address == args.address:
      matches.append(entry)
    elif args.title and entry.title == args.title:
      matches.append(entry)
    elif args.first_name and entry.first_name == args.first_name:
      matches.append(entry)
    elif args.middle_name and entry.middle_name == args.middle_name:
      matches.append(entry)
    elif args.last_name and entry.last_name == args.last_name:
      matches.append(entry)
    elif args.workplace and entry.workplace == args.workplace:
      matches.append(entry)
    elif args.phone and entry.phone == args.phone:
      matches.append(entry)
  return matches
