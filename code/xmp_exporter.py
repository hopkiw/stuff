#!/usr/bin/env python3

import argparse
from libxmp import XMPMeta
from libxmp.consts import XMP_NS_DC as NS
import os

from tagdb import TagDB, getmd5sum

ARRAY_NAME = 'subject'


def get_tags(xmp):
    tags = []
    for i in range(xmp.count_array_items(NS, ARRAY_NAME)):
        tags.append(xmp.get_array_item(NS, ARRAY_NAME, i + 1))

    return tags


def get_xmp(path):
    xmp = XMPMeta()
    if os.path.exists(path):
        with open(path, 'r') as fh:
            buf = fh.read()
        xmp.parse_from_str(buf)

    return xmp


def update_xmp(path, tags):
    xmp = get_xmp(path)
    existing = get_tags(xmp)

    if set(existing) == set(tags):
        return

    xmp.delete_property(NS, ARRAY_NAME)

    for tag in tags:
        xmp.append_array_item(
            NS, ARRAY_NAME, tag, {'prop_value_is_array': 1, 'prop_array_is_unordered': 1})

    print('writing', path)
    with open(path, 'w') as fh:
        fh.write(xmp.serialize_to_unicode())


def get_xmp_files():
    dirn = '/home/cc/pictures/'
    res = []
    for file in os.listdir(dirn):
        if file.endswith('.xmp'):
            res.append((dirn + file, 'pictures/' + file.removesuffix('.xmp')))

    return res


def export_xmp_files():
    db = TagDB()

    res = db.get_images_and_tags()

    imagetags = {}
    for path, tag in res:
        if path not in imagetags:
            imagetags[path] = []
        imagetags[path].append(tag)

    for path, tags in imagetags.items():
        tags = list(filter(None, tags))  # TODO: why are there sometimes empty tags?
        xmppath = path + ".xmp"
        update_xmp(xmppath, tags)


def import_xmp_files():
    image_to_tag_list = []

    xmp_files = get_xmp_files()
    for tagfile, imagefile in xmp_files:
        tags = get_tags(tagfile)
        for tag in tags:
            image_to_tag_list.append((tag, imagefile))

    alltags = set()
    allimages = []
    for tag, image in image_to_tag_list:
        alltags.add((tag,))
        allimages.append((image, getmd5sum(image)))

    db = TagDB()

    print(f'creating {len(alltags)} tags')
    db.add_tag_list(alltags)

    print(f'creating {len(allimages)} images')
    db.add_image_list(allimages)

    print(f'tagging {len(xmp_files)} files')
    db.add_image_tag_list(image_to_tag_list)


def main():
    parser = argparse.ArgumentParser()
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument('-i', '--import', dest='importing', action='store_true')
    group.add_argument('-e', '--export', dest='exporting', action='store_true')
    args = parser.parse_args()

    if args.importing:
        print('Importing tags from xmp files')
        import_xmp_files()

    if args.exporting:
        print('Exporting tags to xmp files')
        export_xmp_files()

    print('All done.')


if __name__ == '__main__':
    main()
