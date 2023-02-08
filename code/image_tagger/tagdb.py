import argparse
import os
import sqlite3
# import sys


# TODO: cache known tags so we don't have to keep checking
# TODO: support add/remove multiple tags at a time
DEF_FILENAME = 'tutorial.db'


class TagDB:
  def __init__(self, filename=None):
    if filename:
      self.filename = filename
    else:
      self.filename = DEF_FILENAME

    create = False
    if not os.path.exists(self.filename):
      create = True

    self.con = sqlite3.connect(self.filename)
    if create:
      print('new db, creating tables')
      self.create_tables()

  def create_tables(self):
    cur = self.con.cursor()
    cur.execute("""
      CREATE TABLE images(
                          imageId INTEGER PRIMARY KEY,
                             path UNIQUE)""")
    cur.execute("""
      CREATE TABLE tags(
                        tagId INTEGER PRIMARY KEY,
                          tag UNIQUE)""")
    cur.execute("""
      CREATE TABLE imagetags(
                             imageId REFERENCES images(imageId),
                               tagId REFERENCES tags(tagId),
                              UNIQUE (imageId, tagId))""")

  def get_tags(self, image):
    cur = self.con.cursor()
    q = """
        SELECT tags.tag
          FROM imagetags
    INNER JOIN tags ON imagetags.tagId = tags.tagId
         WHERE imagetags.imageId = (
                                    SELECT imageId
                                      FROM images
                                     WHERE path = ?)"""

    res = cur.execute(q, (image,))

    return [x[0] for x in res.fetchall()]

  def add_tag(self, image, tag):
    cur = self.con.cursor()

    q = 'INSERT INTO tags (tag) VALUES(?)'
    try:
      cur.execute(q, (tag,))
    except sqlite3.IntegrityError:
      pass
    else:
      self.con.commit()

    q = 'INSERT INTO images (path) VALUES(?)'
    try:
      cur.execute(q, (image,))
    except sqlite3.IntegrityError:
      pass
    else:
      self.con.commit()

    q = """
      INSERT INTO imagetags
           VALUES(
                  (
                   SELECT imageId
                     FROM images
                    WHERE path = ?),
                  (
                   SELECT tagId
                     FROM tags
                    WHERE tag = ?))"""
    try:
      cur.execute(q, (image, tag))
    except sqlite3.IntegrityError as e:
      print('tag binding exists', e)
    else:
      self.con.commit()

  def remove_tag(self, image, tag):
    cur = self.con.cursor()
    q = """
      DELETE FROM imagetags
            WHERE (
                   imageId = (
                              SELECT imageId
                                FROM images
                               WHERE path = ?) AND
                     tagId = (
                              SELECT tagId
                                FROM tags
                               WHERE tag = ?))"""
    cur.execute(q, (image, tag))
    self.con.commit()

  def show(self):
    cur = self.con.cursor()
    res = cur.execute("""
      SELECT images.path,
             tags.tag
        FROM imagetags
  INNER JOIN images ON imagetags.imageId = images.imageId
  INNER JOIN tags ON imagetags.tagId = tags.tagId""")

    return res.fetchall()


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--image', required=True)
  parser.add_argument('--tag', action='append')
  parser.add_argument('--show', action='store_true')
  parser.add_argument('--remove', action='store_true')
  args = parser.parse_args()

  db = TagDB()

  if args.show:
    print('All tags:')
    for entry in db.show():
        print('  Entry:', entry)

    return

  if args.remove and not args.tag:
    print('remove all tags not yet supported')
    return

  if args.tag:
    if args.remove:
      db.remove_tag(args.image, args.tag[0])
    else:
      db.add_tag(args.image, args.tag[0])

  print(args.image + ':')
  for tag in db.get_tags(args.image):
    print('  Tag:', tag)

  db.con.close()


if __name__ == '__main__':
    main()
