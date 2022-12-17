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

#define BUFSIZE 800
#define SERVER_PORT 21

// Globals
FtpPath ftpPath;
int sockfd;
int sockFile;
char connectedSlave = 0;
char buf[BUFSIZE];
struct sockaddr_in server_addr;

// Forward Declaration Just For better Documentation
void getIpAddress();
void ftpCreateMessage(char *dest, const char *command, const char *arg);
int ftpReadMessage(int socketFd, char *buf, int size);
void ftpOpenControlSocket();
void ftpLogIn();
void ftpEnterPassiveMode();
int ftpGetNewPortNumber();
int ftpConnectDownloadSocket(int port);
void ftpSendRetr();
void ftpSendList();

int ftpInit(FtpPath *path);
int ftpQuit();

int ftpInit(FtpPath *path) {
  memcpy(&ftpPath, path, sizeof(FtpPath));
  getIpAddress();
  ftpOpenControlSocket();
  ftpLogIn();
  ftpEnterPassiveMode();
  int port = ftpGetNewPortNumber();
  ftpConnectDownloadSocket(port);
  if (ftpPath.isDir) {
    ftpSendList();
  } else {
    ftpSendRetr();
  }
  return sockFile;
}

int ftpQuit() {
  write(sockfd, "quit\n", strlen("quit\n"));
  print_reply("quit\n");
  ftpSafeReadMessage(sockfd, buf, BUFSIZE);
  if (close(sockfd) < 0) {
    perror("Error while in close()");
    exit(-1);
  }
  if (connectedSlave && close(sockFile) < 0) {
    perror("Error while in close()");
    exit(-1);
  }
  return 1;
}

void getIpAddress() {
  struct hostent *h = gethostbyname(ftpPath.host);
  if (NULL == h) {
    printf("Error getting the ip: %s\n", hstrerror(h_errno));
    exit(-1);
  }
  char *server_ip = inet_ntoa(*((struct in_addr *)h->h_addr));
#ifdef DEBUG
  printf("Ip address %s\n", server_ip);
#endif /* !def DEBUG */
  /*server address handling*/
  bzero((char *)&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr =
      inet_addr(server_ip); /*32 bit Internet address network byte ordered*/
  server_addr.sin_port =
      htons(SERVER_PORT); /*server TCP port must be network byte ordered */
}

void ftpOpenControlSocket() {
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

  ftpSafeReadMessage(sockfd,buf,BUFSIZE);

  // Tenho que checar aqui outra coisa!
  if (buf[0] != '2') {
    perror("Request not completed");
    exit(-1);
  }
}

void ftpLogIn() {
  ftpCreateMessage(buf, "user ", ftpPath.user);
  print_reply("%s\n", buf);
  write(sockfd, buf, strlen(buf));
  ftpSafeReadMessage(sockfd, buf, BUFSIZE);

  ftpCreateMessage(buf, "pass ", ftpPath.password);
  print_reply("%s\n", buf);
  write(sockfd, buf, strlen(buf));
  ftpSafeReadMessage(sockfd, buf, BUFSIZE);
}

void ftpEnterPassiveMode() {
  write(sockfd, "pasv\n", strlen("pasv\n"));
  print_communication("%spasv\n\n", NICEPRINT);
  print_reply("pasv\n\n");
  ftpSafeReadMessage(sockfd, buf, BUFSIZE);
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
  int port = fstArg * 256 + sndArg;
#ifdef DEBUG
  print_communication("Calculating port using (%d,%d)\n", fstArg, sndArg);
  print_communication("Port is : %d\n", port);
#endif /* DEBUG */
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
  return sockFile;
}

void ftpSendRetr() {
  ftpCreateMessage(buf, "retr ", ftpPath.path);
  print_reply("%s\n", buf);
  write(sockfd, buf, strlen(buf));
  // Recieving Status Response
  ftpSafeReadMessage(sockfd, buf, BUFSIZE);
  if (buf[0] > '2') {
    sockFile = -1;
  } else {
    connectedSlave = 1;
  }
}
void ftpSendList() {
  ftpCreateMessage(buf, "list ", ftpPath.path);
  print_reply("%s\n", buf);
  write(sockfd, buf, strlen(buf));
  // Recieving Status Response
  ftpSafeReadMessage(sockfd, buf, BUFSIZE);
}
