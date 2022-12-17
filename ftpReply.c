#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "ftpReply.h"

int READING_MULTILINE = 0;
int READING_SINGLE_LINE = 0;
int WAITING_CR = 0;
int WAITING_NL = 0;
char MULTILINE_CODE[3];

void ftpCreateMessage(char *dest, const char *command, const char *arg) {
  dest[0] = 0;
  strcat(dest, command);
  strcat(dest, arg);
  strcat(dest, "\n");
}
int ftpReadSingleLine(int socketFd, char *buf, int size) {
  int i = 0;
  WAITING_CR = 1;
  WAITING_NL = 1;
  for (; i < size && WAITING_NL; i++) {
    read(socketFd, buf + i, 1);
    if (buf[i] == '\r') {
      WAITING_CR = 0;
    }
    if (buf[i] == '\n') {
      WAITING_NL = 0;
    }
  }
  if (!WAITING_CR && !WAITING_NL && (i <= size)) {
    return -1;
  }
  return i;
}
int ftpReadMultipleLines(int socketFd, char *buf, int size) {
  read(socketFd, buf, 3);
  char newCode[4];
  strncpy(newCode, buf, 3);
  newCode[3] = 0;
  read(socketFd, buf + 3, 1);
  if (buf[3] != '-') {
    if (strcmp(newCode, MULTILINE_CODE) == 0) {
      READING_MULTILINE = 0;
    }
  }
  char gotCariage = 0;
  char ended = 0;
  int i = 4;
  char startReadingCode = 0;
  char code[4];
  int code_i = 0;
  for (; i < size && !ended; i++) {
    read(socketFd, buf + i, 1);
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
        if (strcmp(code, MULTILINE_CODE) == 0) {
          READING_MULTILINE = 0;
          WAITING_CR = 1;
        }
      }
    }
  }
  // Read all the message
  if (ended && !READING_MULTILINE) {
    return -1;
  }
  // There is still reading left
  return i;
}

int ftpReadMessage(int socketFd, char *buf, int size) {
  memset(buf, 0, size);
  int bytesRead = -1;
  if (WAITING_CR || READING_SINGLE_LINE) {
    bytesRead = ftpReadSingleLine(socketFd, buf, size);
  } else if (WAITING_NL) {
    read(socketFd, buf, 1);
    if (buf[0] != '\n') {
      printf("Expected to read newline got: %c", buf[0]);
      exit(1);
    }
  } else if (!READING_MULTILINE && !READING_SINGLE_LINE) {
    read(socketFd, buf, 4);
    if (buf[3] == '-') {
      READING_MULTILINE = 1;
      strncpy(MULTILINE_CODE, buf, 3);
      bytesRead = ftpReadMultipleLines(socketFd, buf + 4, size - 3);
    } else {
      READING_SINGLE_LINE = 1;
      bytesRead = ftpReadSingleLine(socketFd, buf + 4, size - 3);
    }
  } else if (READING_MULTILINE) {
    bytesRead = ftpReadMultipleLines(socketFd, buf, size);
  }
  return bytesRead;
}

void ftpSafeReadMessage(int sockfd, char *buffer, int size) {
  READING_MULTILINE = 0;
  READING_SINGLE_LINE = 0;
  WAITING_CR = 0;
  WAITING_NL = 0;
  int bytesRead;
  do {
    bytesRead = ftpReadMessage(sockfd, buffer, size);
    print_communication("%s",buffer);
  } while (bytesRead != -1);
}
