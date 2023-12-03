#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <gif_lib.h>
#include <iostream>

using namespace std;

static int write(GifFileType *gif, const GifByteType *buf, int len) {
  // cout << "write called" << endl;
  return fwrite(buf, 1, (size_t)len, (FILE *)gif->UserData);
}

int main() {
  int fh = open("myinput.gif", O_RDONLY);
  if (fh == -1) {
    perror("error opening input gif");
    return 1;
  }

  int err;
  GifFileType *gif = DGifOpenFileHandle(fh, &err);
  if (DGifSlurp(gif) != GIF_OK) {
    cout << "GIF_ERR" << endl;
    return 1;
  } else {
    cout << "GIF_OK" << endl;
  }

  close(fh);


  fh = open("myoutput.gif", O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
  if (fh == -1) {
    perror("error opening myoutput.gif");
    return 1;
  }
  FILE *f = fdopen(fh, "wb");

  GifFileType *res;

  res = EGifOpen(f, &write, &err);
  if (res == NULL) {
    cout << "error opening output file" << endl;
    return 1;
  }

  GifMakeSavedImage(res, gif->SavedImages);
  GifDrawBox(res->SavedImages, 10, 10, 20, 20, 1);

  res->SWidth = gif->SWidth;
  res->SHeight = gif->SHeight;
  res->SColorResolution = gif->SColorResolution;
  res->SBackGroundColor = gif->SBackGroundColor;
  res->AspectByte = gif->AspectByte;
  res->SColorMap = gif->SColorMap;
  res->ImageCount = 1;
  res->ExtensionBlockCount = gif->ExtensionBlockCount;
  res->ExtensionBlocks = gif->ExtensionBlocks;

  res->SavedImages->ExtensionBlockCount = gif->SavedImages->ExtensionBlockCount;
  res->SavedImages->ExtensionBlocks = gif->SavedImages->ExtensionBlocks;
  res->SavedImages->ImageDesc.Left = gif->SavedImages->ImageDesc.Left;
  res->SavedImages->ImageDesc.Top = gif->SavedImages->ImageDesc.Top;
  res->SavedImages->ImageDesc.Width = gif->SavedImages->ImageDesc.Width;
  res->SavedImages->ImageDesc.Height = gif->SavedImages->ImageDesc.Height;
  res->SavedImages->ImageDesc.Interlace = gif->SavedImages->ImageDesc.Interlace;
  res->SavedImages->ImageDesc.ColorMap = gif->SavedImages->ImageDesc.ColorMap;

  cout << "SColorResolution: " << res->SColorResolution << endl;;
  cout << "SColorMap->ColorCount: " << res->SColorMap->ColorCount << endl;;
  if (res->SColorMap != NULL) {
    GifColorType *ptr = res->SColorMap->Colors;
    for (int i = 0; i < res->SColorMap->ColorCount; i++) {
      cout << "RGB " << i << ": "
      << static_cast<int>(ptr->Red) << ","
      << static_cast<int>(ptr->Green) << ","
      << static_cast<int>(ptr->Blue) << endl;
      ptr++;
    }
  }

  GifByteType* src = gif->SavedImages->RasterBits;
  int pixels = gif->SWidth * gif->SHeight;
  cout << "Print " << gif->SWidth << "x" << gif->SHeight << " = "
    << pixels << " pixels" << endl;
  for (int i = 0; i < pixels; i++) {
    cout << static_cast<int>(*src) % 10;
    if (i > 0 && (i % res->SWidth == 0)) {
      cout << '\n';
    }
    src++;
  }
  cout << endl;

  EGifSpew(res);

  fflush(f);
  fclose(f);
}
