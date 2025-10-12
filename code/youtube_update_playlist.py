#!/usr/bin/env python3

import arparse
import sys

from google_auth_oauthlib.flow import InstalledAppFlow
import googleapiclient.discovery


scopes = ['https://www.googleapis.com/auth/youtube.force-ssl']
client_secrets_file = 'client_secret.json'


def create_playlist(ytclient, title):
    return ytclient.playlists().insert(
        part='snippet',
        body={
            'snippet': {
                'title': title
            }
        }
    ).execute()


def add_playlist_item(ytclient, playlist_id, video_id):
    return youtube.playlistItems().insert(
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
    grp.add_argument('--title')
    grp.add_argument('--playlist-id')
    args = parser.parse_args()


    # Get credentials and create an API client
    flow = InstalledAppFlow.from_client_secrets_file(client_secrets_file, scopes)
    credentials = flow.run_local_server(port=0)

    ytclient = googleapiclient.discovery.build('youtube', 'v3', credentials=credentials)
    if args.title:
        res = create_playlist(ytclient, title)
        if res.get('kind') == 'youtube#playlist':
            playlist_id = res.get('id')
        else:
            print('Error creating new playlist:', res)
            return
    else:
        playlist_id = args.playlist_id

    with open('urls', 'r') as fh:
        video_ids = fh.read().splitlines()

    for video_id in video_ids:
        print(add_playlist_item(ytclient, video_id))


if __name__ == '__main__':
  main()
