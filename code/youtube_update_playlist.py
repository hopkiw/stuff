#!/usr/bin/env python3

import argparse
import sys

from google_auth_oauthlib.flow import InstalledAppFlow
import googleapiclient.discovery


scopes = ['https://www.googleapis.com/auth/youtube.force-ssl']
client_secrets_file = 'client_secret.json'


class YTClient:
    def __init__(self, credentials):
        self.client = googleapiclient.discovery.build('youtube', 'v3', credentials=credentials)

    def create_playlist(self, title):
        return self.client.playlists().insert(
            part='snippet',
            body={
                'snippet': {
                    'title': title
                }
            }
        ).execute()

    def add_playlist_item(self, playlist_id, video_id):
        return self.client.playlistItems().insert(
            part='snippet',
            body={
                'snippet': {
                    'playlistId': playlist_id,
                    'resourceId': {
                        'kind': 'youtube#video',
                        'videoId': url
                    }
                }
            }
        ).execute()


def main():
    parser = argparse.ArgumentParser()
    grp = parser.add_mutually_exclusive_group(required=True)
    grp.add_argument('--title', help='the title to use for a new playlist')
    grp.add_argument('--playlist-id', help='the ID of an existing playlist')
    parser.add_argument('urls', help='a file of youtube video IDs')
    args = parser.parse_args()

    # Get credentials and create an API client
    flow = InstalledAppFlow.from_client_secrets_file(client_secrets_file, scopes)
    credentials = flow.run_local_server(port=0)

    ytclient = YTClient(credentials)

    if args.title:
        res = ytclient.create_playlist(title)
        if res.get('kind') == 'youtube#playlist':
            playlist_id = res.get('id')
        else:
            print('Error creating new playlist:', res)
            return
    else:
        playlist_id = args.playlist_id

    with open(args.urls, 'r') as fh:
        video_ids = fh.read().splitlines()

    for video_id in video_ids:
        print(ytclient.add_playlist_item(playlist_id, video_id))


if __name__ == '__main__':
  main()
