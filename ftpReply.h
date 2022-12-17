#ifndef __FTPREPLY__
#define __FTPREPLY__

#define NICEPRINT "------> "

#ifdef PRINT_COMMUNICATION
#define print_communication(...) printf(__VA_ARGS__)
#define print_reply(...)                                                       \
  printf("%s", NICEPRINT);                                                     \
  printf(__VA_ARGS__)
#else
#define print_communication(...)
#define print_reply(...)
#endif

void ftpSafeReadMessage(int sockfd,char* buf,int size);
void ftpCreateMessage(char *dest, const char *command, const char *arg);

#endif // !__FTPREPLY__

