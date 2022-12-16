# RCOM-Download
This repository holds the code for RCOM's second project.

# Usage

- `$ make` vai criar o executável `download`

- Listar o diretório de um servidor ftp.
```bash
./download ftp://[user:password@]host/url/para/diretorio 
```
![exemplo-list]()
- Fazer download de um ficheiro num servidor ftp.
```bash
./download ftp://[user:password@]host/url/para/ficherio [novo-nome-para-o-ficheiro]
```
![exemplo-download]()

# Interface de ftpClient

`ftpPath.h` define a api para dar parse a um URL de um servidor ftp:

```c
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
```
A nossa aplicacao de download `download.c` inclui o nosso cliente de ftp que
expoem esta seguinte interface:

```c
#include "ftpPath.h"
int ftpInit(FtpPath *ftpPath);
int ftpQuit();
```

- `ftpInit(FtpPath *ftpPath)`: recebe um apontador para um FtpPath que é o
  resultado de dar parse de um URL FTP.A função implementa um cliente de FTP em
  modo passivo e retorna o file descriptor do socket de leitura.
  O que vai ler do socket depende do URL que lhe passamos:
  - Url de um ficheiro: o conteúdo do ficheiro.
  - Url de um diretório: o conteudo do diretório.
  Obtendo assim flexibilidade de dar `ls` de um diretório no servidor bem como fazer o download de um ficheiro do mesmo.

- `ftpQuit()`: fecha a conexao ftp enviando `quit` para o socket de controlo e fecha os dois sockets

Isto é tudo que um usuário do nosso cliente de ftp,
precisa de saber pois o resto é abstraido e está contido em ficheiros de implementacao (`.c`) que
contem a maior parte da complexidade do trabalho.

# Download 

Como providenciamos esta interface tão simples o seu uso acaba por ser bastante legível e idiomático:

![](download.png)

Nestas 30 linhas de código:
- Damos parse à string que contem o URL 
- Obtemos o fileDescriptor de onde vamos ler
- Se estivermos a trabalhar com um diretório damos print no terminal do seu conteudo.
- Se estivermos a trabalhar com um ficheiro criamos um ficheiro com o nome
  especificado no URL ou com outro nome que o user passe como argumento.

## ftpPath

## ftpClient
