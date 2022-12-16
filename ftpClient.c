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
#define NICEPRINT "------> "

#ifdef PRINT_COMMUNICATION 
#define print_communication(...) printf(__VA_ARGS__ ) 
#else
#define print_communication(...)
#endif

// Globals
FtpPath ftpPath;
int READING_MULTILINE = 0;
char MULTILINE_CODE[3];
int sockfd;
int sockFile;
char buf[BUFSIZE];
struct sockaddr_in server_addr;

void getIpAddress();
void ftpCreateMessage(char *dest, const char *command, const char *arg);
int ftpReadMessage(int socketFd, char *buf, int size);
void ftpOpenControlSocket();
void ftpLogIn();
void ftpEnterPassiveMode();
int ftpGetNewPortNumber();
int ftpConnectDownloadSocket(int port);

int ftpInit(FtpPath *path,enum FtpAction action);
int ftpQuit();

void ftpCreateMessage(char *dest, const char *command, const char *arg) {
  dest[0] = 0;
  strcat(dest, command);
  strcat(dest, arg);
  strcat(dest, "\n");
}
int ftpReadMessage(int socketFd, char *buf, int size) {
  memset(buf, 0, size);
  buf[3] = 0;
  read(socketFd, buf, 3);
  if (READING_MULTILINE && strcmp(buf, MULTILINE_CODE)) {
    READING_MULTILINE = 0;
  }
  read(socketFd, buf + 3, 1);
  print_communication("%s", buf);
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
    print_communication("%c", buf[i]);
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
  struct hostent *h = gethostbyname(ftpPath.host);
  if( NULL == h){
     printf("Error getting the ip: %s\n",hstrerror(h_errno));
     exit(-1);
    }
  char* server_ip = inet_ntoa(*((struct in_addr *)h->h_addr));
  print_communication("Ip address %s\n", server_ip);
  /*server address handling*/
  bzero((char *)&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr =
      inet_addr(server_ip); /*32 bit Internet address network byte ordered*/
  server_addr.sin_port =
      htons(SERVER_PORT); /*server TCP port must be network byte ordered */

}

void ftpOpenControlSocket(){
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

  ftpReadMessage(sockfd, buf, BUFSIZE);
  if (buf[0] != '2') {
    perror("Request not completed");
    exit(-1);
  }
}
void ftpLogIn() {
  ftpCreateMessage(buf, "user ", ftpPath.user);
  print_communication("%s%s\n",NICEPRINT, buf);
  write(sockfd, buf, strlen(buf));
  ftpReadMessage(sockfd, buf, BUFSIZE);

  ftpCreateMessage(buf, "pass ", ftpPath.password);
  print_communication("%s%s\n",NICEPRINT, buf);
  write(sockfd, buf, strlen(buf));
  ftpReadMessage(sockfd, buf, BUFSIZE);
}

void ftpEnterPassiveMode() {
  write(sockfd, "pasv\n", strlen("pasv\n"));
  print_communication("%spasv\n\n",NICEPRINT);
  ftpReadMessage(sockfd, buf, BUFSIZE);
  
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
  print_communication("Calculating port using (%d,%d)\n", fstArg, sndArg);
  int port = fstArg * 256 + sndArg;
  print_communication("Port is : %d\n", port);
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
  ftpCreateMessage(buf, "retr ", ftpPath.path);
  print_communication("%s%s\n",NICEPRINT,buf);
  write(sockfd, buf, strlen(buf));
  // Recieving Status Response
  ftpReadMessage(sockfd, buf, BUFSIZE);
  return sockFile;
}

int ftpInit(FtpPath *path,enum FtpAction action){
    memcpy(&ftpPath,path,sizeof(FtpPath));
    getIpAddress();
    ftpOpenControlSocket();
    ftpLogIn();
    ftpEnterPassiveMode();
    int port = ftpGetNewPortNumber();
    if(action == FtpDownload){
    ftpConnectDownloadSocket(port);
    }
    else if (action == FtpList){
        print_communication("Implementing Listing Action\n");
    }
    return sockFile;
}

int ftpQuit() {
  write(sockfd,"quit\n",strlen("quit\n"));
  print_communication("%s%s",NICEPRINT,"quit\n");
  ftpReadMessage(sockfd,buf,BUFSIZE);
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
