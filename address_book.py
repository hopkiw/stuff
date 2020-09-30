#!/usr/bin/python
import sys
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
	def __init__(self, name, email, phone, mailing1, mailing2, street1, street2):
		self.name = name
		self.email = email
		self.phone = phone
		self.mailing1 = mailing1
		self.mailing2 = mailing2
		self.street1 = street1
		self.street2 = street2

	def info(self):
		print(self.name)
		if self.email:
			print('Email Address: ' + self.email)
		if self.phone:
			print('Phone Number: ' + self.phone)
		if self.mailing1 and self.mailing2:
			print('Mailing Address: ' + self.mailing1)
			print('                 ' + self.mailing2)
		if self.street1 and self.street2:
			print('Street Address:  ' + self.street1)
			print('                 ' + self.street2)


def main():
	parser = argparse.ArgumentParser()
	parser.add_argument('--add',action='store_true',help='Action to be performed on the address book.')
	parser.add_argument('--name',help='Action to be performed on the address book.')
	parser.add_argument('--email',help='Action to be performed on the address book.')
	parser.add_argument('--phone',help='Action to be performed on the address book.')
	parser.add_argument('--mailing1',help='Action to be performed on the address book.')
	parser.add_argument('--mailing2',help='Action to be performed on the address book.')
	parser.add_argument('--street1',help='Action to be performed on the address book.')
	parser.add_argument('--street2',help='Action to be performed on the address book.')

	parser.add_argument('--delete',action='store_true',help='Action to be performed on the address book.')

	parser.add_argument('--search',action='store_true',help='Action to be performed on the address book.')
	parser.add_argument('--searchterm',help='Action to be performed on the address book.')
	
	args = parser.parse_args()

	if args.add:
		if args.delete or args.search:
			print('Choose one action only.')
			exit()
		if not args.name:
			print('Name is a required field.')
			exit()
		new_entry = Entry(args.name,args.email,args.phone,args.mailing1,args.mailing2,args.street1,args.street2)
		entries.append(new_entry)
		new_entry.info()

	if args.delete:
		if args.add or args.search:
			print('Choose one action only.')
			exit()
		to_delete=[]
		for n in range(0, len(entries)):
			if args.name != entries[n].name:
				continue			
			print(entries[n].info())
			action = raw_input('Is this the entry you wish to delete? (Y/N)').upper()
			if action == 'Y':
				entries.pop(n)

#	if args.name or args.email:
#		new_entry = Entry(args.name,args.email,args.phone,args.mailing1,args.mailing2,args.street1,args.street2)
#		entries.append(new_entry)
#		print(new_entry.info())
	if args.search:
		found = []
		for entry in entries:
			if args.searchterm in entry.name or args.searchterm in entry.email or args.searchterm in entry.phone or args.searchterm in entry.mailing1 or args.searchterm in entry.mailing2 or args.searchterm in entry.street1 or args.searchterm in entry.street2:
				found.append(entry)
		for entry in found:
			entry.info()

if __name__ == "__main__":
	main()
