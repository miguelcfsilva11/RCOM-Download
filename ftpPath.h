#ifndef __FTPARGUMENT__
#define __FTPARGUMENT__

#define ARG_SIZE 400
#define DEFAULT_ARG "ftp://ftp.up.pt/debian/README.html"
#define PROTOCOL_SIZE 6
#define PROTOCOL "ftp://"

typedef struct {
  char protocol[50];
  char host[100];
  char path[100];
  char password[50];
  char user[50];
  int auth;
} FtpPath;

void printFtpPath(FtpPath* ftpPath);
int parseFTPPath(const char* ftpString, FtpPath* ftpPath);

#endif // !__FTPARGUMENT__
