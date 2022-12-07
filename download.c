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

#include "ftpArgument.c"

#define BUFSIZE 1000 
#define SERVER_PORT 21

int READING_MULTILINE = 0;
char MULTILINE_CODE[3];

int read_message(int socketFd, char *buf, int size) {
  buf[3] = 0;
  read(socketFd, buf, 3);
  // printf("Code: %s!\n", buf);
  if (READING_MULTILINE && strcmp(buf, MULTILINE_CODE)) {
    READING_MULTILINE = 0;
  }
  read(socketFd, buf + 3, 1);
  printf("%s", buf);
  if (buf[3] == '-') {
    READING_MULTILINE = 1;
  }
  char gotCariage = 0;
  char ended = 0;
  int i = 4;
  char startReadingCode = 0;
  char code[4];
  int code_i = 0;
  for (; i < size && !ended; i++) {
    read(socketFd, buf + i, 1);
    printf("%c", buf[i]);
    if (buf[i] == '\r') {
      gotCariage = 1;
    }
    if (buf[i] == '\n') {
      if (gotCariage && !READING_MULTILINE) {
        ended = 1;
        gotCariage = 0;
      }
      startReadingCode = 1;
    } else if (startReadingCode) {
      code[code_i] = buf[i];
      code_i++;
      if (code_i == 4) {
        startReadingCode = 0;
        code_i = 0;
        if (code[3] == '-') {
          continue;
        }
        code[3] = 0;
        if (strcmp(code, MULTILINE_CODE)) {
          READING_MULTILINE = 0;
        }
      }
    }
  }
  if (ended && !READING_MULTILINE) {
    return -1;
  }
  // There is still reading left
  if (READING_MULTILINE) {
    printf("\nMultiline Reply\n");
  }
  if (i >= size) {
    printf(
        "There is still reading left to do, message doesnt fit this buffer!\n");
  }
  return i;
}

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

  int sockfd;
  struct sockaddr_in server_addr;
  char buf[BUFSIZE];

  struct hostent *h;

  //---------------------------
  // Translating Host Name To IP Address
  //---------------------------
  h = gethostbyname(ftpPath.host);
  char *server_ip = inet_ntoa(*((struct in_addr *)h->h_addr));
  printf("Ip address %s\n", server_ip);
  /*server address handling*/
  bzero((char *)&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr =
      inet_addr(server_ip); /*32 bit Internet address network byte ordered*/
  server_addr.sin_port =
      htons(SERVER_PORT); /*server TCP port must be network byte ordered */

  /*open a TCP socket*/
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Error while in socket()");
    exit(-1);
  }

  /*connect to the server*/
  if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    perror("Error while in connect()");
    exit(-1);
  }

  printf("Here\n");
  // recv(sockfd, buf, BUFSIZE, 0);
  read_message(sockfd, buf, BUFSIZE);
  printf("Server sent %s\n", buf);
  if (buf[0] != '2') {
    perror("Request not completed");
    exit(-1);
  }
  // ---------------------------
  //    Establishing Connection
  // ---------------------------
  memset(buf, 0, BUFSIZE);
  strcpy(buf, "user ");
  strcat(buf, ftpPath.user);
  strcat(buf, "\n");
  printf("Message is %s\n", buf);
  write(sockfd, buf, strlen(buf));
  memset(buf, 0, BUFSIZE);
  // recv(sockfd, buf, BUFSIZE, 0);
  read_message(sockfd, buf, BUFSIZE);
  printf("Server Sent %s\n", buf);

  memset(buf, 0, BUFSIZE);
  strcpy(buf, "pass ");
  strcat(buf, ftpPath.password);
  strcat(buf, "\n");
  printf("Message is %s\n", buf);
  write(sockfd, buf, strlen(buf));
  memset(buf, 0, BUFSIZE);
  // recv(sockfd, buf, BUFSIZE, 0);
  read_message(sockfd, buf, BUFSIZE);
  printf("Server Sent %s\n", buf);

  memset(buf, 0, BUFSIZE);
  write(sockfd, "pasv\n", strlen("pasv\n"));
  printf("We wrote pasv%s\n", buf);
  // recv(sockfd, buf, BUFSIZE, 0);
  read_message(sockfd, buf, BUFSIZE);
  printf("Server Sent %s\n", buf);

  // ---------------------------
  //    Calculating The Port
  // ---------------------------

  int commaCounter = 0;
  int fstArg = 0;
  int sndArg = 0;
  for (size_t i = 0; i < strlen(buf) && buf[i] != ')'; i++) {
    if (buf[i] == ',') {
      commaCounter++;
      continue;
    }
    if (commaCounter == 4) {
      fstArg *= 10;
      fstArg += (buf[i] - '0');

    } else if (commaCounter == 5) {
      sndArg *= 10;
      sndArg += (buf[i] - '0');
    }
  }
  printf("Calculating port using (%d,%d)\n", fstArg, sndArg);
  int port = fstArg * 256 + sndArg;
  printf("Port is : %d\n", port);

  int sockFile;
  if ((sockFile = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Error while in socket()");
    exit(-1);
  }

  server_addr.sin_port = htons(port);

  /*connect to the server*/
  if (connect(sockFile, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    perror("Error while in connect()");
    exit(-1);
  }

  //-------------------------------
  //      Requesting The File
  //-------------------------------
  memset(buf, 0, BUFSIZE);
  strcat(buf, "retr ");
  strcat(buf, ftpPath.path);
  strcat(buf, "\n");
  write(sockfd, buf, strlen(buf));
  // Recieving Status Response
  memset(buf, 0, BUFSIZE);
  read_message(sockfd, buf, BUFSIZE);
  printf("Server Sent %s\n", buf);

  memset(buf, 0, BUFSIZE);
  FILE *file = fopen(filename, "wb");

  int bytes = -1;
  do {
    bytes = read(sockFile, buf, BUFSIZE);
    fwrite(buf, strlen(buf), 1, file);
    memset(buf, 0, BUFSIZE);
  } while (bytes >0);

  if (close(sockfd) < 0) {
    perror("Error while in close()");
    exit(-1);
  }

  if (close(sockFile) < 0) {
    perror("Error while in close()");
    exit(-1);
  }
  return 0;
}
