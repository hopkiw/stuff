#!/usr/bin/env python3

from google_auth_oauthlib.flow import InstalledAppFlow
import googleapiclient.discovery

scopes = ["https://www.googleapis.com/auth/youtube.force-ssl"]
client_secrets_file = "client_secret.json"


def main():
  if len(sys.argv) < 2:
      print(f'Usage: {sys.argv[0]} <playlist id>')

  playlist = sys.argv[1]

  # Get credentials and create an API client
  flow = InstalledAppFlow.from_client_secrets_file(client_secrets_file, scopes)
  credentials = flow.run_local_server(port=0)

  youtube = googleapiclient.discovery.build("youtube", "v3", credentials=credentials)


  with open('urls') as fh:
      urls = fh.read().splitlines()


  for url in urls:
      request = youtube.playlistItems().insert(
          part="snippet",
          body={
              "snippet": {
                  "playlistId": playlist,
                  "resourceId": {
                    "kind": "youtube#video",
                    "videoId": url
                  }
              }
          }
      )

      response = request.execute()
      print(response)


if __name__ == '__main__':
  main()
