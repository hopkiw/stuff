import os
import sqlite3
import sys


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

  def all_tags(self):
    res = self.show()
    print(res)


def main():
  if len(sys.argv[1:]) == 0:
    print('Usage:', sys.argv[0], '<image> [tag ...]')
    sys.exit(0)

  image = sys.argv[1]
  tags = sys.argv[2:]

  db = TagDB()

  for tag in tags:
    db.add_tag(image, tag)

  print(image + ':')
  for i, tag in enumerate(db.get_tags(image)):
    print('\tTag %2d: %s' % (i, tag))

  db.con.close()


if __name__ == '__main__':
    main()
