#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARG_SIZE 100
#define DEFAULT_ARG "ftp://ftp.up.pt/pub/kodi/timestamp.txt"
#define PROTOCOL_SIZE 6
#define PROTOCOL "ftp://"

typedef struct {
  char protocol[50];
  char host[50];
  char path[50];
  char password[50];
  char user[50];
  int auth;
} FtpPath;

void printFtpPath(FtpPath* ftpPath) {
  printf("protocol:%s\n", ftpPath->protocol);
  printf("host:%s\n", ftpPath->host);
  printf("path:%s\n", ftpPath->path);
  if (ftpPath->auth) {
    printf("password:%s\n", ftpPath->password);
    printf("user:%s\n", ftpPath->user);
  }
}

int checkProtocol(const char* ftpString, FtpPath* ftpPath) {
  char protocol[PROTOCOL_SIZE + 1];
  strncpy(protocol, ftpString, PROTOCOL_SIZE);
  protocol[PROTOCOL_SIZE] = 0;
  if (strcmp(protocol, PROTOCOL)) {
    printf("Wrong protocol expected %s, got %s\n", PROTOCOL, protocol);
    return 0;
  }
  strncpy(ftpPath->protocol, protocol, PROTOCOL_SIZE + 1);
  return 1;
}

int parseFTPPathDefault(const char* ftpString, FtpPath* ftpPath, int start) {
  int i = start;
  int j = 0;
  int gotHost = 0;
  while (!gotHost) {
    if (ftpString[i] == '/') {
      gotHost = 1;
      ftpPath->host[i] = 0;
    } else {
      ftpPath->host[j] = ftpString[i];
      j++;
    }
    i++;
  }
  strcpy(ftpPath->path, ftpString + i);
  return 1;
}
int parseFTPPathAuth(const char* ftpString, FtpPath* ftpPath, int atLocation) {
  int j = 0;
  int gettingUser = 1;
  for (int i = PROTOCOL_SIZE; i < atLocation; i++, j++) {
    if (ftpString[i] == ':' && gettingUser) {
      gettingUser = 0;
      ftpPath->user[j] = 0;
      j = -1;
    } else if (gettingUser) {
      ftpPath->user[j] = ftpString[i];
    } else {
      ftpPath->password[j] = ftpString[i];
    }
  }
  return parseFTPPathDefault(ftpString, ftpPath, atLocation + 1);
}
int parseFTPPath(const char* ftpString, FtpPath* ftpPath) {
  if (!checkProtocol(ftpString, ftpPath)) {
    return 0;
  }
  int authenticated = 0;
  int i = PROTOCOL_SIZE;
  for (; i < ARG_SIZE; i++) {
    if (ftpString[i] == '@') {
      authenticated = 1;
      break;
    }
    if (ftpString[i] == '/') {
      break;
    }
  }
  if (authenticated) {
    ftpPath->auth = 1;
    return parseFTPPathAuth(ftpString, ftpPath, i);
  } else {
    ftpPath->auth = 0;
    strcpy(ftpPath->user,"anonymous");
    strcpy(ftpPath->password,"larinha");
    return parseFTPPathDefault(ftpString, ftpPath, PROTOCOL_SIZE);
  }
}

// int main(int argc, char* argv[]) {
// char ftpArgument[ARG_SIZE + 1];
// if (argc < 2) {
// printf("Usage ftpArgument ftp://[user:password@]host/path\n");
// strncpy(ftpArgument, DEFAULT_ARG, ARG_SIZE);
// // ftpArgument[ARG_SIZE]=0;
// } else {
// strncpy(ftpArgument, argv[1], ARG_SIZE);
// ftpArgument[ARG_SIZE] = 0;
// }
// FtpPath path;
// FtpPath* ftpPath = &path;
// printf("Hello world! arg is : %s\n", ftpArgument);
// if (parseFTPPath(ftpArgument, ftpPath)) {
// printFtpPath(ftpPath);
// } else {
// printf("Error while parsing FTP path\n");
// }

// return 0;
// }
