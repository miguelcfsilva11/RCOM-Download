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

#define SERVER_PORT 21

int main(int argc, char **argv) {
  //----------------------------
  //    Getting The ftp URL
  //----------------------------
  char ftpArgument[ARG_SIZE + 1];
  if (argc < 2) {
    printf("ftp://ftp.up.pt/pub/kodi/timestamp.txt");
    strncpy(ftpArgument, DEFAULT_ARG, ARG_SIZE);
    // ftpArgument[ARG_SIZE]=0;
  } else {
    strncpy(ftpArgument, argv[1], ARG_SIZE);
    ftpArgument[ARG_SIZE] = 0;
  }
  FtpPath ftpPath;
  printf("Hello world! arg is : %s\n", ftpArgument);

  //----------------------------
  //    Parsing the URL
  //----------------------------
  if (parseFTPPath(ftpArgument, &ftpPath)) {
    printFtpPath(&ftpPath);
    fflush(stdout);
  } else {
    printf("Error while parsing FTP path\n");
  }
  // -----------------------------
  //   Getting the File Name
  // -----------------------------
  char filename[51];
  int i = 0;
  int filenameStart=0; 
  while (ftpPath.path[i] != 0) {
    if (ftpPath.path[i] == '/') {
      filenameStart = i+1;
    }
    i++;
  }
  strncpy(filename,ftpPath.path + filenameStart,50);
  filename[50]=0;
  printf("Got filename : %si\n",filename);
  printf("filenameStart is :%d",filenameStart);


  int sockfd;
  struct sockaddr_in server_addr;
  char buf[BUFSIZ];

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

  recv(sockfd, buf, BUFSIZ, 0);
  printf("Server sent %s\n", buf);
  if (buf[0] != '2') {
    perror("Request not completed");
    exit(-1);
  }
  // ---------------------------
  //    Establishing Connection
  // ---------------------------
  memset(buf, 0, BUFSIZ);
  strcpy(buf, "user ");
  strcat(buf, ftpPath.user);
  strcat(buf, "\n");
  printf("Message is %s\n", buf);
  write(sockfd, buf, strlen(buf));
  memset(buf, 0, BUFSIZ);
  recv(sockfd, buf, BUFSIZ, 0);
  printf("Server Sent %s\n", buf);

  memset(buf, 0, BUFSIZ);
  strcpy(buf, "pass ");
  strcat(buf, ftpPath.password);
  strcat(buf, "\n");
  printf("Message is %s\n", buf);
  write(sockfd, buf, strlen(buf));
  memset(buf, 0, BUFSIZ);
  recv(sockfd, buf, BUFSIZ, 0);
  printf("Server Sent %s\n", buf);

  memset(buf, 0, BUFSIZ);
  write(sockfd, "pasv\n", strlen("pasv\n"));
  printf("We wrote pasv%s\n", buf);
  recv(sockfd, buf, BUFSIZ, 0);
  printf("Server Sent %s\n", buf);

  // ---------------------------
  //    Established Connection
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
  memset(buf, 0, BUFSIZ);
  strcat(buf, "retr ");
  strcat(buf, ftpPath.path);
  strcat(buf, "\n");
  write(sockfd, buf, strlen(buf));
  // Recieving Status Response
  memset(buf, 0, BUFSIZ);
  recv(sockfd, buf, BUFSIZ, 0);
  printf("Server Sent %s\n", buf);

  memset(buf, 0, BUFSIZ);
  FILE *file = fopen(filename, "wb");

  while (1) {
    recv(sockFile, buf, BUFSIZ, 0);
    if (strlen(buf) == 0)
      break;
    fwrite(buf, strlen(buf), 1, file);
    memset(buf, 0, BUFSIZ);
  }

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
