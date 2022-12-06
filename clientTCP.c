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
  if (parseFTPPath(ftpArgument, &ftpPath)) {
    printFtpPath(&ftpPath);
    fflush(stdout);
  } else {
    printf("Error while parsing FTP path\n");
  }
  int sockfd;
  struct sockaddr_in server_addr;
  char buf[BUFSIZ];
  size_t bytes;

  struct hostent *h;
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

  /*send a string to the server*/
  // Esta string não tem nada?
  bytes = write(sockfd, buf, strlen(buf));
  if (bytes > 0)
    printf("Bytes escritos %ld\n", bytes);
  else {
    perror("Error while in write()");
    exit(-1);
  }

  if (close(sockfd) < 0) {
    perror("Error while in close()");
    exit(-1);
  }

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Error while in socket()");
    exit(-1);
  }

  return 0;
}
