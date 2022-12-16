#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "ftpClient.h"

#define DOWNLOAD_BUF_SIZE 8000
#define DEFAULT_ARG "ftp://ftp.up.pt/debian/"

int main(int argc, char **argv) {
  char ftpArgument[ARG_SIZE + 1];
  if (argc < 2) {
    printf("Nothing Specified usage is :\n");
    printf("download ftp://[user:password]@host/url [newfilename]\n");
    printf("Using Defaults....\n");
    printf("%s\n", DEFAULT_ARG);
    strncpy(ftpArgument, DEFAULT_ARG, ARG_SIZE);
    // ftpArgument[ARG_SIZE]=0;
  } else {
    strncpy(ftpArgument, argv[1], ARG_SIZE);
    ftpArgument[ARG_SIZE] = 0;
  }

  FtpPath ftpPath;
  if (parseFTPPath(ftpArgument, &ftpPath)) {
    printFtpPath(&ftpPath);
  } else {
    printf("Error while parsing FTP path\n !Shuting Down!");
    exit(1);
  }
  int bytes = -1;
  char downloadBuf[DOWNLOAD_BUF_SIZE];
  int sockFile = ftpInit(&ftpPath);

  if (ftpPath.isDir) {
    do {
      bytes = read(sockFile, downloadBuf, DOWNLOAD_BUF_SIZE);
      printf("%s", downloadBuf);
    } while (bytes > 0);
  }

  else {
    printf("Got filename : %s\n", ftpPath.fileName);
    FILE *file = fopen(ftpPath.fileName, "wb");
    do {
      bytes = read(sockFile, downloadBuf, DOWNLOAD_BUF_SIZE);
      fwrite(downloadBuf, bytes, 1, file);
    } while (bytes > 0);
  }

  ftpQuit();
}
