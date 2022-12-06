/**      (C)2000-2021 FEUP
 *       tidy up some includes and parameters
 * */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "ftpArgument.c"

#define SERVER_PORT 6000
#define SERVER_ADDR "192.168.28.96"

int main(int argc, char **argv) {
  char ftpArgument[ARG_SIZE + 1];
  if (argc < 2) {
    printf("Usage ftpArgument ftp://[user:password@]host/path\n");
    strncpy(ftpArgument, DEFAULT_ARG, ARG_SIZE);
    // ftpArgument[ARG_SIZE]=0;
  } else {
    strncpy(ftpArgument, argv[1], ARG_SIZE);
    ftpArgument[ARG_SIZE] = 0;
  }
  FtpPath ftpPath;
  printf("Hello world! arg is : %s\n", ftpArgument);
  if (parseFTPPath(ftpArgument, &ftpPath)) {
    printFtpPath(&ftpPath);
  } else {
    printf("Error while parsing FTP path\n");
  }

  int sockfd;
  struct sockaddr_in server_addr;
  char buf[BUFSIZ];
  size_t bytes;

  /*server address handling*/
  bzero((char *)&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr =
      inet_addr(SERVER_ADDR); /*32 bit Internet address network byte ordered*/
  server_addr.sin_port =
      htons(SERVER_PORT); /*server TCP port must be network byte ordered */

  /*open a TCP socket*/
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket()");
    exit(-1);
  }

  /*connect to the server*/
  if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    perror("connect()");
    exit(-1);
  }

  recv(sockfd, buf, BUFSIZ, 0);
  if (buf[0] != '2') {
    perror("Request not completed");
    exit(-1);
  }

  buf = strcat("user ", user);
  write(sockfd, buf, strlen(buf));
  recv(sockfd, buf, BUFSIZ, 0);

  buf = strcat("pass ", password);
  write(sockfd, buf, strlen(buf));
  recv(sockfd, buf, BUFSIZ, 0);

  write(sockfd, "pasv", strlen("pasv"));

  /*send a string to the server*/
  bytes = write(sockfd, buf, strlen(buf));
  if (bytes > 0)
    printf("Bytes escritos %ld\n", bytes);
  else {
    perror("write()");
    exit(-1);
  }

  if (close(sockfd) < 0) {
    perror("close()");
    exit(-1);
  }
  return 0;
}
