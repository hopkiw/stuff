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
	args = parser.parse_args()
	new_entry = Entry(args.name,args.email)
	entries.append(new_entry)

if __name__ == "__main__":
	main()
