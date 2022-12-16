#ifndef __FTPARGUMENT__
#define __FTPARGUMENT__

#define ARG_SIZE 550
#define PROTOCOL_SIZE 6
#define PROTOCOL "ftp://"

typedef struct {
  char protocol[50];
  char host[200];
  char path[200];
  char password[50];
  char user[50];
  char fileName[100];
  char isDir;
  char auth;
} FtpPath;

void printFtpPath(FtpPath* ftpPath);
int parseFTPPath(const char* ftpString, FtpPath* ftpPath);

#endif // !__FTPARGUMENT__
