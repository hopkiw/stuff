#!/usr/bin/env python3
"""emby stuff"""

import argparse
import json
import requests

URL='http://10.100.50.201:8096'
API_KEY='4e918c4ae034400e9d16ea3b802e0053'
USER_ID='d7b36ca114c346adb3c3f0da47e98c00'


def get_items(parent_id, extra=None):
    """get_items"""
    path=f'/Users/{USER_ID}/Items'
    params={'api_key': API_KEY, 'parentId': str(parent_id)}
    if extra:
        params.update(extra)

    ret = requests.get(URL + path, params=params, timeout=5)
    return ret.json()


def get_views():
    """get_views"""
    path = f'/Users/{USER_ID}/Views'
    params={'api_key': API_KEY}

    ret = requests.get(URL + path, params=params, timeout=5)
    return ret.json()


def get_shows(extra=None):
    """get_shows"""
    params={'Fields': 'People'}
    if extra:
        params.update(extra)

    return get_items(6, params)


def get_guest_stars(show_id):
    """print_guest_stars"""
    seasons = get_items(show_id)
    params = {'Fields': 'People'}

    res = []
    for season in seasons['Items']:
        episodes = get_items(season['Id'], params)
        for episode in episodes['Items']:
            gs = []
            people = episode.get('People')
            if not people:
                continue
            for person in people:
                if person['Type'] == 'GuestStar':
                    gs.append(person['Name'])

            if gs:
                res.append(episode['Name'], gs)

    return res


def get_movies(extra=None):
    """query"""
    params={}
    if extra:
        params.update(extra)

    return get_items(4, params)


def get_collections():
    """get_collections"""
    return get_items(145326)


def get_movie_years():
    """get_movie_years"""
    params={'Fields': 'ProductionYear'}
    movies = get_movies(params)

    res = []
    for movie in movies['Items']:
        if not movie:
            continue
        year = movie.get('ProductionYear', 0)
        res.append((movie['Name'], year))

    return res


def get_movie_directors():
    """directors"""
    params={'Fields': 'People'}
    movies = get_movies(params)

    res = []
    for movie in movies['Items']:
        if not movie:
            continue
        people = movie.get('People')
        if not people:
            continue
        for person in people:
            typ = person.get('Type')
            if typ == 'Director':
                res.append((movie['Name'], person['Name']))

    return res


def get_movie_actors():
    """get_movie_actors"""
    params={'Fields': 'People'}
    movies = get_movies(params)

    res = []
    for movie in movies['Items']:
        if not movie:
            continue
        people = movie.get('People')
        if not people:
            continue
        actors = []
        for person in people:
            typ = person.get('Type')
            if typ == 'Actor':
                actors.append(person['Name'])
        res.append((movie['Name'], actors))

    return res


def main():
    """main"""
    parser = argparse.ArgumentParser()
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument('-y', '--years', action='store_true')
    group.add_argument('-d', '--directors', action='store_true')
    group.add_argument('-a', '--actors', action='store_true')
    group.add_argument('-m', '--movies', action='store_true')
    group.add_argument('-s', '--shows', action='store_true')
    group.add_argument('-v', '--views', action='store_true')
    group.add_argument('-c', '--collections', action='store_true')
    args = parser.parse_args()

    res = []
    if args.years:
        res = get_movie_years()
    elif args.directors:
        res = get_movie_directors()
    elif args.actors:
        res = get_movie_actors()
    elif args.movies:
        res = get_movies()
    elif args.shows:
        res = get_shows()
    elif args.views:
        res = get_views()
    elif args.collections:
        res = get_collections()

    print(json.dumps(res))


if __name__ == '__main__':
    main()
