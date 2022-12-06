#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define ARG_SIZE 100
#define DEFAULT_ARG "ftp://user:password@host/foo/bar.txt"
#define PROTOCOL_SIZE 6
#define PROTOCOL "ftp://"

typedef struct{
    char protocol[50];
    char host[50];
    char path[50];
    char password[50];
    char user[50];
} FtpPath;

int checkProtocol(const char* ftpString,FtpPath* ftpPath){
    char protocol[PROTOCOL_SIZE+1];
    strncpy(protocol,ftpString,PROTOCOL_SIZE);
    protocol[PROTOCOL_SIZE] = 0;
    if(strcmp(protocol,PROTOCOL)){
        printf("Wrong protocol expected %s, got %s\n",PROTOCOL,protocol);
        return 0;
    }
    strncpy(ftpPath->protocol,protocol,PROTOCOL_SIZE+1);
    return 1;

}
int parseFTPPath(const char* ftpString,FtpPath* ftpPath){
    if(!checkProtocol(ftpString,ftpPath)){
        return 0;
    }
    int authenticated = 0;
    for(int i = PROTOCOL_SIZE;i< ARG_SIZE;i++){
        if(ftpString[i] == '@'){
            authenticated= 1;
            break;
        }
        if(ftpString[i] == '/'){
            break;
        }
    }
    if (authenticated){
        return parseFTPPathAuth(ftpString,ftpPath);
    }
    else{
        return parseFTPPathDefault(ftpString,ftpPath);
    }

}
int main(int argc, char* argv[]){
    char ftpArgument[ARG_SIZE+1];
    if (argc < 2){
        printf("Usage ftpArgument ftp://[user:password@]host/path\n");
        strncpy(ftpArgument,DEFAULT_ARG,ARG_SIZE);
        //ftpArgument[ARG_SIZE]=0;
    }
    else{
        strncpy(ftpArgument,argv[1],ARG_SIZE);
        ftpArgument[ARG_SIZE]=0;
    }
    printf("Hello world! arg is : %s\n",ftpArgument);
    return 0;
}
