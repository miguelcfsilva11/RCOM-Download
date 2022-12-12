#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdarg.h>
#include "ftpClient.h"

#define BUFSIZE 1000
#define SERVER_PORT 21

FtpPath ftpPath;
int READING_MULTILINE = 0;
char MULTILINE_CODE[3];
int sockfd;
int sockFile;
struct sockaddr_in server_addr;
char buf[BUFSIZE];
struct hostent *h;
char *server_ip;

void create_message(char *dest, const char *command, const char *arg) {
  // memset(dest,0,BUFSIZE);
  dest[0] = 0;
  strcat(dest, command);
  strcat(dest, arg);
  strcat(dest, "\n");
}
int read_message(int socketFd, char *buf, int size) {
  memset(buf, 0, size);
  buf[3] = 0;
  read(socketFd, buf, 3);
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

void getIpAddress() {
  h = gethostbyname(ftpPath.host);
  printf("Aqui está bem\n");
  fflush_unlocked(stdout);
  server_ip = inet_ntoa(*((struct in_addr *)h->h_addr));
  printf("Aqui está bem\n");
  fflush_unlocked(stdout);
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

  read_message(sockfd, buf, BUFSIZE);
  printf("Server sent %s\n", buf);
  if (buf[0] != '2') {
    perror("Request not completed");
    exit(-1);
  }
}

void ftpLogIn() {
  create_message(buf, "user ", ftpPath.user);
  printf("Message is %s\n", buf);
  write(sockfd, buf, strlen(buf));
  read_message(sockfd, buf, BUFSIZE);
  printf("Server Sent %s\n", buf);

  create_message(buf, "pass ", ftpPath.password);
  printf("Message is %s\n", buf);
  write(sockfd, buf, strlen(buf));
  read_message(sockfd, buf, BUFSIZE);
  printf("Server Sent %s\n", buf);
}

void ftpEnterPassiveMode() {
  write(sockfd, "pasv\n", strlen("pasv\n"));
  printf("We wrote pasv%s\n", buf);
  read_message(sockfd, buf, BUFSIZE);
  printf("Server Sent %s\n", buf);
  
}

int ftpGetNewPortNumber() {
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
  return port;
}

int ftpConnectDownloadSocket(int port) {
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
  create_message(buf, "retr ", ftpPath.path);
  write(sockfd, buf, strlen(buf));
  // Recieving Status Response
  read_message(sockfd, buf, BUFSIZE);
  printf("Server Sent %s\n", buf);

  return sockFile;
}

int ftpInit(FtpPath *path){
    memcpy(&ftpPath,path,sizeof(FtpPath));
    printFtpPath(&ftpPath);
    getIpAddress();
    ftpLogIn();
    ftpEnterPassiveMode();
    int port = ftpGetNewPortNumber();
    ftpConnectDownloadSocket(port);
    return sockFile;
}

int ftpQuit() {
  if (close(sockfd) < 0) {
    perror("Error while in close()");
    exit(-1);
  }
  if (close(sockFile) < 0) {
    perror("Error while in close()");
    exit(-1);
  }
  return 1;
}
