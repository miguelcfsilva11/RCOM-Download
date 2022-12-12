/**      (C)2000-2021 FEUP
 *       tidy up some includes and parameters
 * */

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "ftpClient.h"

#include <stdarg.h>


#define DOWNLOAD_BUF_SIZE 1000

int main(int argc, char **argv) {
  //----------------------------
  //    Getting The ftp URL
  //----------------------------
  char ftpArgument[ARG_SIZE + 1];
  if (argc < 2) {
    printf("Nothing Specified usage is :\n");
    printf("download ftp://[user:password]host/url [newfilename]\n");
    printf("Using Defaults....\n");
    printf("%s\n", DEFAULT_ARG);
    strncpy(ftpArgument, DEFAULT_ARG, ARG_SIZE);
    // ftpArgument[ARG_SIZE]=0;
  } else {
    strncpy(ftpArgument, argv[1], ARG_SIZE);
    ftpArgument[ARG_SIZE] = 0;
  }
  printf("Hello world! arg is : %s\n", ftpArgument);

  //----------------------------
  //    Parsing the URL
  //----------------------------
  FtpPath ftpPath;
  if (parseFTPPath(ftpArgument, &ftpPath)) {
    printFtpPath(&ftpPath);
  } else {
    printf("Error while parsing FTP path\n !Shuting Down!");
    exit(1);
  }
  // -----------------------------
  //   Getting the File Name
  // -----------------------------
  char filename[51];
  if (argc <= 2) {
    int i = 0;
    int filenameStart = 0;
    while (ftpPath.path[i] != 0) {
      if (ftpPath.path[i] == '/') {
        filenameStart = i + 1;
      }
      i++;
    }
    strncpy(filename, ftpPath.path + filenameStart, 50);
    filename[50] = 0;
  } else {
    printf("Will it break here ??%s\n", argv[2]);
    strncpy(filename, argv[2], 50);
    filename[50] = 0;
  }

  printf("Got filename : %s\n", filename);

  int sockFile = ftpInit(&ftpPath);

  FILE *file = fopen(filename, "wb");

  int bytes = -1;
  char downloadBuf[DOWNLOAD_BUF_SIZE];
  do {
    bytes = read(sockFile,downloadBuf,DOWNLOAD_BUF_SIZE);
    fwrite(downloadBuf,bytes, 1, file);
  } while (bytes >0);

  return 0;
  ftpQuit();
}
