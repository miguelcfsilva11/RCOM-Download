CC=gcc
FLAGS= -Wall -Werror
OUT=download

all: normal

normal:
	$(CC)  $(FLAGS) -D PRINT_COMMUNICATION ftpPath.c ftpClient.c download.c -o $(OUT)

debug:
	$(CC)  $(FLAGS) -D DEBUG -D PRINT_COMMUNICATION ftpPath.c ftpClient.c download.c -o $(OUT)


