#!/usr/bin/python

import argparse

# entries will be stored somewhere... need to get them out
# if there are no entries stored yet, need to handle that case too


#placeholder until i write something to store and retrieve entries
entries = []


#name, email, addresses, online presence, labels, notes, photo??
#think extensibility
#delete entry
#edit entry

class Entry(object):
	def __init__(self, name, email):
		self.name = name
		self.email = email

	def info(self):
		print(self.name + ': ' + self.email)

def main():
	parser = argparse.ArgumentParser()
	parser.add_argument('--name',help='The name of the entry to add.')
	parser.add_argument('--email',help='The email address of the entry to add.')
	parser.add_argument('--search',help='The term to search for.')
	args = parser.parse_args()
	if args.name or args.email:
		new_entry = Entry(args.name,args.email)
		entries.append(new_entry)
		print(new_entry.info())
	if args.search:
		found = []
		for entry in entries:
			if args.search in entry.name or args.search in entry.email:
				found.append(entry)
		for entry in found:
			entry.info()

if __name__ == "__main__":
	main()
