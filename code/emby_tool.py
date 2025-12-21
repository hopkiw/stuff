#!/usr/bin/env python3
"""emby stuff"""

import argparse
import configparser
import json
import os
import requests


class EmbyTool:
    def __init__(self, url, api_key, user_id):
        self.url = url
        self.api_key = api_key
        self.user_id = user_id

    def get_items(self, parent_id, extra=None):
        """get_items"""
        path = f'/Users/{self.user_id}/Items'
        params = {'api_key': self.api_key, 'parentId': str(parent_id)}
        if extra:
            params.update(extra)

        ret = requests.get(self.url + path, params=params, timeout=5)
        return ret.json()

    def get_views(self):
        """get_views"""
        path = f'/Users/{self.user_id}/Views'
        params = {'api_key': self.api_key}

        ret = requests.get(self.url + path, params=params, timeout=5)
        return ret.json()

    def get_shows(self, extra=None):
        """get_shows"""
        params = {'Fields': 'People'}
        if extra:
            params.update(extra)

        return self.get_items(6, params)

    def get_guest_stars(self, show_id):
        """print_guest_stars"""
        seasons = self.get_items(show_id)
        params = {'Fields': 'People'}

        res = []
        for season in seasons['Items']:
            episodes = self.get_items(season['Id'], params)
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

    def get_movies(self, extra=None):
        """query"""
        params = {}
        if extra:
            params.update(extra)

        return self.get_items(4, params)

    def get_collections(self, ):
        """get_collections"""
        return self.get_items(145326)

    def get_movie_years(self):
        """get_movie_years"""
        params = {'Fields': 'ProductionYear'}
        movies = self.get_movies(params)

        res = []
        for movie in movies['Items']:
            if not movie:
                continue
            year = movie.get('ProductionYear', 0)
            res.append((movie['Name'], year))

        return res

    def get_movie_directors(self):
        """directors"""
        params = {'Fields': 'People'}
        movies = self.get_movies(params)

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

    def get_movie_actors(self):
        """get_movie_actors"""
        params = {'Fields': 'People'}
        movies = self.get_movies(params)

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

    config = configparser.ConfigParser()
    config.read(os.path.join(os.environ.get('HOME'), '.config/emby.conf'))

    emby = EmbyTool(config['whatever']['url'], config['whatever']['api_key'], config['whatever']['user_id'])

    res = []
    if args.years:
        res = emby.get_movie_years()
    elif args.directors:
        res = emby.get_movie_directors()
    elif args.actors:
        res = emby.get_movie_actors()
    elif args.movies:
        res = emby.get_movies()
    elif args.shows:
        res = emby.get_shows()
    elif args.views:
        res = emby.get_views()
    elif args.collections:
        res = emby.get_collections()

    print(json.dumps(res))


if __name__ == '__main__':
    main()
