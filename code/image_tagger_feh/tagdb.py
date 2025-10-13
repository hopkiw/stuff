#!/usr/bin/env python3


# TODO: image_path should be whole path or not
# TODO: get image by ANY tag (loop over bind to create IN (?, ?, ...))


import argparse
import hashlib
import sqlite3
import sys


class TagDB:
    def __init__(self):
        self.con = sqlite3.connect("/home/cc/.config/tagger/tagdb.sqlite3")
        cur = self.con.cursor()
        cur.execute("""
        CREATE TABLE IF NOT EXISTS tags (
            id  INTEGER PRIMARY KEY,
            tag TEXT UNIQUE NOT NULL
        )""")
        cur.execute("""
        CREATE TABLE IF NOT EXISTS images (
            id         INTEGER PRIMARY KEY,
            image_path TEXT NOT NULL,
            image_hash TEXT NOT NULL,
            UNIQUE(image_path, image_hash)
        )""")
        cur.execute("""
        CREATE TABLE IF NOT EXISTS imagetags (
            id       INTEGER PRIMARY KEY,
            image_id INTEGER NOT NULL,
            tag_id   INTEGER NOT NULL,
            FOREIGN KEY(image_id) REFERENCES images(id),
            FOREIGN KEY(tag_id) REFERENCES tags(id),
            UNIQUE(image_id, tag_id)
        )""")
        self.con.commit()

    def add_image_tag(self, image_id, tag_id):
        cur = self.con.cursor()
        cur.execute('INSERT OR IGNORE INTO imagetags (image_id, tag_id) VALUES (?, ?)', (image_id, tag_id))
        self.con.commit()

    def add_image_tag_list(self, image_list, tag_id):
        cur = self.con.cursor()
        cur.executemany("""
            INSERT OR IGNORE INTO imagetags (image_id, tag_id) SELECT id, ?
            FROM images
            WHERE image_path = ? AND image_hash = ?""",
                        [(tag_id, image_path, image_hash) for image_path, image_hash in image_list])
        self.con.commit()

    def add_tag(self, tag):
        cur = self.con.cursor()
        cur.execute('INSERT OR IGNORE INTO tags (tag) VALUES (?)', (tag,))
        self.con.commit()

    def add_image(self, image_path, image_hash):
        cur = self.con.cursor()
        cur.execute('INSERT OR IGNORE INTO images (image_path, image_hash) VALUES (?, ?)',
                    (image_path, image_hash))
        self.con.commit()

    def add_image_list(self, image_list):
        cur = self.con.cursor()
        cur.executemany('INSERT OR IGNORE INTO images (image_path, image_hash) VALUES (?, ?)', image_list)
        self.con.commit()

    def get_tag(self, tag):
        cur = self.con.cursor()
        res = cur.execute('SELECT id FROM tags WHERE tag = ?', (tag,))
        maybe = res.fetchone()

        return maybe[0] if maybe else None

    def get_image_by_path(self, image_path):
        cur = self.con.cursor()
        res = cur.execute('SELECT id FROM images WHERE image_path = ?', (image_path,))
        maybe = res.fetchone()

        return maybe[0] if maybe else None

    def get_image_by_hash(self, image_hash):
        cur = self.con.cursor()
        res = cur.execute('SELECT id FROM images WHERE image_hash = ?', (image_hash,))
        maybe = res.fetchone()

        return maybe[0] if maybe else None

    def get_images_by_tag(self, tag_id):
        cur = self.con.cursor()
        res = cur.execute("""
              SELECT image_path
              FROM images
              LEFT JOIN imagetags ON images.id = imagetags.image_id
              WHERE tag_id = ?""", (tag_id,))

        return [entry[0] for entry in res.fetchall()]

    def get_tags_by_image(self, image_id):
        cur = self.con.cursor()
        res = cur.execute("""
              SELECT tag
              FROM tags
              LEFT JOIN imagetags ON tags.id = imagetags.tag_id
              WHERE image_id = ?""", (image_id,))

        return [entry[0] for entry in res.fetchall()]

    def get_all_tags(self):
        cur = self.con.cursor()
        res = cur.execute('SELECT tag FROM tags')

        return [entry[0] for entry in res.fetchall()]

    def get_images_all_tags(self, tags):
        cur = self.con.cursor()
        sql = """
              SELECT image_path
              FROM images i
              """

        for i, tag in enumerate(tags, 1):
            sql += f"""
              INNER JOIN (
                  SELECT image_id
                  FROM imagetags it
                  JOIN tags t
                  ON t.id = it.tag_id
                  WHERE tag = ?
                ) t{i}
              ON i.id = t{i}.image_id
              """
        res = cur.execute(sql, tuple(tags))

        return [entry[0] for entry in res.fetchall()]


def getmd5sum(path):
    with open(path, 'rb') as fh:
        s = fh.read()
    return hashlib.md5(s).hexdigest()


def main():
    # TODO:
    # user should be able to search 'all images that match ALL these tags'
    # user should be able to search 'all images that match ANY of these tags'

    # user should be able to provide a file of filenames to tag
    # user should be able to specify multiple tags to add at once

    # user should be able to rename, merge, or delete tags

    parser = argparse.ArgumentParser()
    grp = parser.add_mutually_exclusive_group(required=True)
    grp.add_argument("--add", action="store_true")
    grp.add_argument("--get", action="store_true")
    grp.add_argument("--get-all-tags", action="store_true")

    grp2 = parser.add_mutually_exclusive_group()
    grp2.add_argument("--path")
    grp2.add_argument("--file")

    parser.add_argument("--tag", nargs='+')
    args = parser.parse_args()

    db = TagDB()

    if args.get_all_tags:
        for tag in db.get_all_tags():
            print(tag)

        return

    if args.get:
        if len(list(filter(None, (args.tag, args.path)))) != 1:
            print('--get requires --tag or --path')
            return

        if args.tag:
            if len(args.tag) == 1:
                tag_id = db.get_tag(args.tag)
                print(f'images tagged "{args.tag}":')
                for image in db.get_images_by_tag(tag_id):
                    print(image)
            else:
                print(f'images with tags: {args.tag}')
                for image in db.get_images_all_tags(args.tag):
                    print(image)

            return 0

        if args.path:
            image_id = db.get_image_by_path(args.path)
            print(f'tags for file "{args.path}":')
            for tag in db.get_tags_by_image(image_id):
                print(tag)

            return 0

    if args.add:
        if not args.tag:
            print('--add requires --tag')
            return 1

        if not args.path and not args.file:
            print('--add requires either --path or --file')
            return 1

        if args.file:
            with open(args.file, 'r') as fh:
                paths = fh.read().splitlines()

        else:
            paths = [args.path]

        db.add_tag(args.tag)
        tag_id = db.get_tag(args.tag)

        images = []
        for path in paths:
            md5sum = getmd5sum(path)
            images.append((path, md5sum))

        db.add_image_list(images)
        db.add_image_tag_list(images, tag_id)

        print('Done tagging.')
        return 0


if __name__ == '__main__':
    sys.exit(main())
