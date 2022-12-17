#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "ftpClient.h"


#define DOWNLOAD_BUF_SIZE 8000
#define DEFAULT_ARG "ftp://ftp.up.pt/debian/"
#define NEW_FILENAME_SIZE 200

int main(int argc, char **argv) {
  char ftpArgument[ARG_SIZE + 1];
  int bytes = -1;
  char downloadBuf[DOWNLOAD_BUF_SIZE + 1];
  char newFileName[NEW_FILENAME_SIZE + 1];
  char *fileNamePtr = NULL;
  if (argc < 2) {
    printf("Nothing Specified usage is :\n");
    printf("download ftp://[user:password]@host/url [newfilename]\n");
    printf("Using Defaults....\n");
    printf("%s\n", DEFAULT_ARG);
    strncpy(ftpArgument, DEFAULT_ARG, ARG_SIZE);
  } else {
    strncpy(ftpArgument, argv[1], ARG_SIZE);
    ftpArgument[ARG_SIZE] = 0;
    if (argc == 3) {
      strncpy(newFileName, argv[2], NEW_FILENAME_SIZE);
      newFileName[NEW_FILENAME_SIZE] = 0;
      fileNamePtr = newFileName;
    }
  }

  FtpPath ftpPath;
  if (!parseFTPPath(ftpArgument, &ftpPath)) {
    printf("Error while parsing FTP path\n !Shuting Down!");
    exit(1);
  }
  #ifdef DEBUG
    printFtpPath(&ftpPath);
  #endif /* DEBUG */

  int sockFile = ftpInit(&ftpPath);
  if(sockFile == -1){
    ftpQuit();
    exit(1);
    }
  if (ftpPath.isDir) {
    do {
      bytes = read(sockFile, downloadBuf, DOWNLOAD_BUF_SIZE);
      printf("%s", downloadBuf);
    } while (bytes > 0);
  }
  else {
    if (fileNamePtr == NULL) {
      fileNamePtr = ftpPath.fileName;
    }

    FILE *file = fopen(fileNamePtr, "wb");
    do {
      bytes = read(sockFile, downloadBuf, DOWNLOAD_BUF_SIZE);
      fwrite(downloadBuf, bytes, 1, file);
    } while (bytes > 0);
  }

  ftpQuit();
}
