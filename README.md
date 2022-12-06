# RCOM-Download
This repository holds the code for RCOM's second project.

## Ports

Mapping between services and ports in FEUP lab's Unix System:

- HTTPS: 80
- SMTP: 25
- FTP: 21
- POP3: 110

## Todo 

Add variadic function to write message to the server
```c
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
```
