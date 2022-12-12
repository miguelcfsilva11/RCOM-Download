#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ftpPath.h"


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
  // Guarantees that the strings will finish
  memset(ftpPath,0,sizeof(FtpPath));
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
